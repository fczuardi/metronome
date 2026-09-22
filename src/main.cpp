#include <Arduino.h>
#include <M5Unified.h>

#include "BeatClock.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t TEMPO_BPM = 120;
constexpr uint8_t BEATS_PER_BAR = 4;
constexpr uint32_t BEAT_INTERVAL_MS = 60000UL / TEMPO_BPM;
constexpr int16_t BEAT_ROW_Y = 82;
constexpr int16_t BEAT_FIRST_X = 57;
constexpr int16_t BEAT_SPACING = 42;
constexpr int16_t BEAT_RADIUS = 10;
constexpr int16_t STATUS_ROW_Y = 113;
constexpr int16_t STATUS_ROW_HEIGHT = 18;

BeatClock beatClock(BEAT_INTERVAL_MS);
uint8_t currentBeat = 0;

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

void drawInputStatus(const char* status) {
  M5.Display.fillRect(0, STATUS_ROW_Y, M5.Display.width(), STATUS_ROW_HEIGHT,
                      TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, STATUS_ROW_Y + 3);
  M5.Display.printf("Input: %s", status);
}

void drawScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.print("METRONOME");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(70, 38);
  M5.Display.printf("%u", TEMPO_BPM);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(132, 53);
  M5.Display.print("BPM");

  for (uint8_t beat = 0; beat < BEATS_PER_BAR; beat++) {
    drawBeat(beat, beat == currentBeat);
  }
  drawInputStatus("ready - silent clock");
}

void reportButtons() {
  if (M5.BtnA.wasPressed()) {
    Serial.println("button: name=a action=pressed");
    drawInputStatus("A pressed");
  }
  if (M5.BtnA.wasReleased()) {
    Serial.println("button: name=a action=released");
    drawInputStatus("A released");
  }
  if (M5.BtnB.wasPressed()) {
    Serial.println("button: name=b action=pressed");
    drawInputStatus("B pressed");
  }
  if (M5.BtnB.wasReleased()) {
    Serial.println("button: name=b action=released");
    drawInputStatus("B released");
  }
}

void advanceVisibleBeat(uint32_t nowMs) {
  const uint32_t elapsed = beatClock.elapsedBeats(nowMs);
  if (elapsed == 0) return;

  const uint8_t previousBeat = currentBeat;
  currentBeat = (currentBeat + elapsed) % BEATS_PER_BAR;
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
  drawScreen();
  beatClock.begin(millis());
  Serial.printf("metronome: silent_clock=ready bpm=%u beats_per_bar=%u\n",
                TEMPO_BPM, BEATS_PER_BAR);
}

void loop() {
  M5.update();
  reportButtons();
  advanceVisibleBeat(millis());
  delay(1);
}
