#include <Arduino.h>
#include <M5Unified.h>

#include "BeatClock.h"
#include "ControlSurface.h"
#include "MetronomeAudio.h"
#include "MetronomeClickSamples.h"
#include "MetronomeDisplay.h"
#include "MetronomeState.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;

MetronomeState state;
BeatClock beatClock(state.beatIntervalMs());
ControlSurface controls;
MetronomeAudio audio;
MetronomeDisplay display;

void adjustTempo(int8_t direction, uint32_t nowMs) {
  const uint16_t previousTempo = state.tempoBpm();
  if (!state.adjustTempo(direction)) return;

  beatClock.setIntervalMs(nowMs, state.beatIntervalMs());
  display.drawControl(state);
  Serial.printf(
      "control: action=adjust mode=tempo direction=%s previous=%u value=%u "
      "interval_ms=%lu phase=preserved now_ms=%lu\n",
      direction > 0 ? "up" : "down", previousTempo, state.tempoBpm(),
      static_cast<unsigned long>(state.beatIntervalMs()),
      static_cast<unsigned long>(nowMs));
}

void adjustVolume(int8_t direction) {
  const uint8_t previousVolume = state.clickVolume();
  if (!state.adjustVolume(direction)) return;

  audio.setVolume(state.clickVolume());
  display.drawControl(state);
  Serial.printf(
      "control: action=adjust mode=volume direction=%s previous=%u value=%u\n",
      direction > 0 ? "up" : "down", previousVolume, state.clickVolume());
}

void selectNextSound(bool accent) {
  state.selectNextSound(accent);
  display.drawControl(state);
  const ClickSoundId selection =
      accent ? state.accentClick() : state.regularClick();
  Serial.printf("control: action=adjust mode=sound role=%s selected=%s\n",
                accent ? "accent" : "regular",
                clickSoundLogName(selection));
}

void applyControl(ControlCommand command, uint32_t nowMs) {
  if (command == ControlCommand::None) return;

  if (command == ControlCommand::NextMode) {
    Serial.println("button: action=chord names=a+b");
    state.cycleControlMode();
    display.drawControl(state);
    Serial.printf("control: action=mode selected=%s\n",
                  state.controlModeName());
    return;
  }

  const int8_t direction =
      command == ControlCommand::Increase ? 1 : -1;
  Serial.printf("button: name=%c action=released command=%s\n",
                direction > 0 ? 'a' : 'b',
                direction > 0 ? "increase" : "decrease");

  switch (state.controlMode()) {
    case ControlMode::Tempo:
      adjustTempo(direction, nowMs);
      break;
    case ControlMode::Volume:
      adjustVolume(direction);
      break;
    case ControlMode::Sound:
      selectNextSound(direction > 0);
      break;
  }
}

void triggerCurrentClick(uint32_t nowMs) {
  const bool downbeat = state.currentBeat() == 0;
  const ClickSoundId sound =
      downbeat ? state.accentClick() : state.regularClick();
  const PcmS8Sample& sample = clickSoundSample(sound);
  const bool started = audio.play(sample);
  const uint32_t durationMs =
      sample.sampleCount * 1000UL / sample.sampleRateHz;
  Serial.printf(
      "click: beat=%u accent=%s sound=%s duration_ms=%lu sample_rate_hz=%lu "
      "samples=%u volume=%u gain=master playback=started ok=%s now_ms=%lu\n",
      state.currentBeat() + 1, downbeat ? "yes" : "no",
      clickSoundLogName(sound), static_cast<unsigned long>(durationMs),
      static_cast<unsigned long>(sample.sampleRateHz),
      static_cast<unsigned>(sample.sampleCount), state.clickVolume(),
      started ? "yes" : "no", static_cast<unsigned long>(nowMs));
}

void advanceVisibleBeat(uint32_t nowMs) {
  const uint32_t elapsed = beatClock.elapsedBeats(nowMs);
  if (elapsed == 0) return;

  const uint8_t previousBeat = state.currentBeat();
  state.advanceBeat(elapsed);
  triggerCurrentClick(nowMs);
  display.drawBeat(previousBeat, false);
  display.drawBeat(state.currentBeat(), true);

  Serial.printf("beat: index=%u elapsed=%lu now_ms=%lu\n",
                state.currentBeat() + 1, static_cast<unsigned long>(elapsed),
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
  buildMetronomeClickSamples();
  const bool keepAliveStarted = audio.begin(state.clickVolume());
  display.drawScreen(state);
  beatClock.begin(millis());
  triggerCurrentClick(millis());
  Serial.printf(
      "metronome: audible_clock=ready bpm=%u volume=%u beats_per_bar=%u "
      "control_mode=%s\n",
      state.tempoBpm(), state.clickVolume(), MetronomeState::BEATS_PER_BAR,
      state.controlModeName());
  Serial.printf(
      "audio: idle_policy=keep_alive ok=%s\n",
      keepAliveStarted ? "yes" : "no");
}

void loop() {
  M5.update();
  const uint32_t nowMs = millis();
  advanceVisibleBeat(nowMs);
  applyControl(controls.poll(), nowMs);
  delay(1);
}
