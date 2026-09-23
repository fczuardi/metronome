#include <Arduino.h>
#include <M5Unified.h>

#include "BeatClock.h"
#include "M5BuzzerToneOutput.h"
#include "MetronomeClickSamples.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t INITIAL_TEMPO_BPM = 120;
constexpr uint16_t MIN_TEMPO_BPM = 30;
constexpr uint16_t MAX_TEMPO_BPM = 300;
constexpr uint8_t BEATS_PER_BAR = 4;
constexpr uint8_t INITIAL_CLICK_VOLUME = 128;
constexpr uint8_t VOLUME_STEP = 8;
constexpr size_t CLICK_SAMPLE_CAPACITY = 2560;
constexpr size_t KEEP_ALIVE_SAMPLE_COUNT = 256;
constexpr int KEEP_ALIVE_CHANNEL = 1;
constexpr int16_t BEAT_ROW_Y = 82;
constexpr int16_t BEAT_FIRST_X = 57;
constexpr int16_t BEAT_SPACING = 42;
constexpr int16_t BEAT_RADIUS = 10;
constexpr int16_t STATUS_ROW_Y = 113;
constexpr int16_t STATUS_ROW_HEIGHT = 18;

enum class ControlMode : uint8_t {
  Tempo,
  Volume,
};

uint32_t beatIntervalMs(uint16_t tempoBpm) {
  return 60000UL / tempoBpm;
}

BeatClock beatClock(beatIntervalMs(INITIAL_TEMPO_BPM));
M5BuzzerToneOutput buzzerOutput;
int8_t scaledClickSamples[CLICK_SAMPLE_CAPACITY];
int8_t keepAliveSilence[KEEP_ALIVE_SAMPLE_COUNT] = {};
uint16_t tempoBpm = INITIAL_TEMPO_BPM;
uint8_t clickVolume = INITIAL_CLICK_VOLUME;
uint8_t currentBeat = 0;
ControlMode controlMode = ControlMode::Tempo;
bool inputArmed = true;

const char* controlModeName() {
  return controlMode == ControlMode::Tempo ? "tempo" : "volume";
}

void drawBeat(uint8_t beat, bool active) {
  const int16_t x = BEAT_FIRST_X + beat * BEAT_SPACING;
  const uint16_t color = beat == 0 ? TFT_YELLOW : TFT_CYAN;
  M5.Display.fillCircle(x, BEAT_ROW_Y, BEAT_RADIUS, TFT_BLACK);
  if (active) {
    M5.Display.fillCircle(x, BEAT_ROW_Y, BEAT_RADIUS, color);
  } else {
    M5.Display.drawCircle(x, BEAT_ROW_Y, BEAT_RADIUS, TFT_DARKGREY);
  }
}

void drawControl() {
  M5.Display.fillRect(0, 32, M5.Display.width(), 40, TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(70, 38);
  if (controlMode == ControlMode::Tempo) {
    M5.Display.printf("%u", tempoBpm);
  } else {
    M5.Display.printf("%u", clickVolume);
  }

  M5.Display.setTextSize(1);
  M5.Display.setCursor(132, 53);
  M5.Display.print(controlMode == ControlMode::Tempo ? "BPM" : "VOL");
}

void drawInputStatus() {
  M5.Display.fillRect(0, STATUS_ROW_Y, M5.Display.width(), STATUS_ROW_HEIGHT,
                      TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, STATUS_ROW_Y + 3);
  M5.Display.printf("%s   A +   B -",
                    controlMode == ControlMode::Tempo ? "TEMPO" : "VOLUME");
}

void drawScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.print("METRONOME");

  drawControl();
  for (uint8_t beat = 0; beat < BEATS_PER_BAR; beat++) {
    drawBeat(beat, beat == currentBeat);
  }
  drawInputStatus();
}

void cycleControlMode() {
  controlMode = controlMode == ControlMode::Tempo ? ControlMode::Volume
                                                  : ControlMode::Tempo;
  drawControl();
  drawInputStatus();
  Serial.printf("control: action=mode selected=%s\n", controlModeName());
}

void adjustTempo(int8_t direction, uint32_t nowMs) {
  const int16_t candidate = static_cast<int16_t>(tempoBpm) + direction;
  const uint16_t bounded =
      static_cast<uint16_t>(constrain(candidate, MIN_TEMPO_BPM, MAX_TEMPO_BPM));
  if (bounded == tempoBpm) return;

  const uint16_t previousTempo = tempoBpm;
  tempoBpm = bounded;
  beatClock.setIntervalMs(nowMs, beatIntervalMs(tempoBpm));
  drawControl();
  Serial.printf(
      "control: action=adjust mode=tempo direction=%s previous=%u value=%u "
      "interval_ms=%lu phase=preserved now_ms=%lu\n",
      direction > 0 ? "up" : "down", previousTempo, tempoBpm,
      static_cast<unsigned long>(beatIntervalMs(tempoBpm)),
      static_cast<unsigned long>(nowMs));
}

void adjustVolume(int8_t direction) {
  const int16_t candidate =
      static_cast<int16_t>(clickVolume) + direction * VOLUME_STEP;
  const uint8_t bounded = static_cast<uint8_t>(constrain(candidate, 0, 255));
  if (bounded == clickVolume) return;

  const uint8_t previousVolume = clickVolume;
  clickVolume = bounded;
  drawControl();
  Serial.printf(
      "control: action=adjust mode=volume direction=%s previous=%u value=%u\n",
      direction > 0 ? "up" : "down", previousVolume, clickVolume);
}

PcmS8Sample renderClickAtVolume(const PcmS8Sample& source) {
  const size_t sampleCount =
      source.sampleCount < CLICK_SAMPLE_CAPACITY ? source.sampleCount
                                                 : CLICK_SAMPLE_CAPACITY;
  const int32_t gainSquared =
      static_cast<int32_t>(clickVolume) * clickVolume;
  constexpr int32_t MAX_GAIN_SQUARED = 255 * 255;

  for (size_t index = 0; index < sampleCount; index++) {
    scaledClickSamples[index] = static_cast<int8_t>(
        static_cast<int32_t>(source.data[index]) * gainSquared /
        MAX_GAIN_SQUARED);
  }

  return {scaledClickSamples, sampleCount, source.sampleRateHz};
}

void adjustCurrentControl(int8_t direction, uint32_t nowMs) {
  if (controlMode == ControlMode::Tempo) {
    adjustTempo(direction, nowMs);
  } else {
    adjustVolume(direction);
  }
}

void handleButtons(uint32_t nowMs) {
  if (!inputArmed) {
    if (!M5.BtnA.isPressed() && !M5.BtnB.isPressed()) inputArmed = true;
    return;
  }

  if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
    Serial.println("button: action=chord names=a+b");
    cycleControlMode();
    inputArmed = false;
    return;
  }

  if (M5.BtnA.wasReleased() && !M5.BtnB.isPressed()) {
    Serial.println("button: name=a action=released command=increase");
    adjustCurrentControl(1, nowMs);
    inputArmed = false;
    return;
  }

  if (M5.BtnB.wasReleased() && !M5.BtnA.isPressed()) {
    Serial.println("button: name=b action=released command=decrease");
    adjustCurrentControl(-1, nowMs);
    inputArmed = false;
  }
}

void triggerCurrentClick(uint32_t nowMs) {
  const bool downbeat = currentBeat == 0;
  const PcmS8Sample& source = metronomeClickSample();
  const PcmS8Sample sample = renderClickAtVolume(source);
  const bool muted = clickVolume == 0;
  const bool started = muted || buzzerOutput.playSample(sample);
  const uint32_t durationMs =
      sample.sampleCount * 1000UL / sample.sampleRateHz;
  Serial.printf(
      "click: beat=%u accent=%s sound=%s duration_ms=%lu sample_rate_hz=%lu "
      "samples=%u volume=%u gain=pcm playback=%s ok=%s "
      "now_ms=%lu\n",
      currentBeat + 1, downbeat ? "yes" : "no", metronomeClickName(),
      static_cast<unsigned long>(durationMs),
      static_cast<unsigned long>(sample.sampleRateHz),
      static_cast<unsigned>(sample.sampleCount), clickVolume,
      muted ? "muted" : "started",
      started ? "yes" : "no", static_cast<unsigned long>(nowMs));
}

void advanceVisibleBeat(uint32_t nowMs) {
  const uint32_t elapsed = beatClock.elapsedBeats(nowMs);
  if (elapsed == 0) return;

  const uint8_t previousBeat = currentBeat;
  currentBeat = (currentBeat + elapsed) % BEATS_PER_BAR;
  triggerCurrentClick(nowMs);
  drawBeat(previousBeat, false);
  drawBeat(currentBeat, true);

  Serial.printf("beat: index=%u elapsed=%lu now_ms=%lu\n", currentBeat + 1,
                static_cast<unsigned long>(elapsed),
                static_cast<unsigned long>(nowMs));
}
}  // namespace

void setup() {
  auto config = M5.config();
  config.internal_spk = true;
  config.internal_mic = false;
  config.fallback_board = m5::board_t::board_M5StickCPlus2;
  M5.begin(config);

  Serial.begin(SERIAL_BAUD);
  delay(200);

  M5.Display.setRotation(1);
  buildMetronomeClickSample();
  buzzerOutput.begin();
  buzzerOutput.setVolume(255);
  const bool keepAliveStarted = M5.Speaker.playRaw(
      keepAliveSilence, KEEP_ALIVE_SAMPLE_COUNT, 16000, false, UINT32_MAX,
      KEEP_ALIVE_CHANNEL, true);
  drawScreen();
  beatClock.begin(millis());
  triggerCurrentClick(millis());
  Serial.printf(
      "metronome: audible_clock=ready bpm=%u volume=%u beats_per_bar=%u "
      "control_mode=%s\n",
      tempoBpm, clickVolume, BEATS_PER_BAR, controlModeName());
  Serial.printf(
      "audio: action=keep_alive channel=%d samples=%u repeat=forever ok=%s\n",
      KEEP_ALIVE_CHANNEL, static_cast<unsigned>(KEEP_ALIVE_SAMPLE_COUNT),
      keepAliveStarted ? "yes" : "no");
}

void loop() {
  M5.update();
  const uint32_t nowMs = millis();
  advanceVisibleBeat(nowMs);
  handleButtons(nowMs);
  delay(1);
}
