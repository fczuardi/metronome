#include "MetronomeDisplay.h"

#include <M5Unified.h>

namespace {
constexpr int16_t BEAT_ROW_Y = 82;
constexpr int16_t BEAT_FIRST_X = 57;
constexpr int16_t BEAT_SPACING = 42;
constexpr int16_t BEAT_RADIUS = 10;
constexpr int16_t STATUS_ROW_Y = 113;
constexpr int16_t STATUS_ROW_HEIGHT = 18;
}  // namespace

void MetronomeDisplay::drawBeat(uint8_t beat, bool active) {
  const int16_t x = BEAT_FIRST_X + beat * BEAT_SPACING;
  const uint16_t color = beat == 0 ? TFT_YELLOW : TFT_CYAN;
  M5.Display.fillCircle(x, BEAT_ROW_Y, BEAT_RADIUS, TFT_BLACK);
  if (active) {
    M5.Display.fillCircle(x, BEAT_ROW_Y, BEAT_RADIUS, color);
  } else {
    M5.Display.drawCircle(x, BEAT_ROW_Y, BEAT_RADIUS, TFT_DARKGREY);
  }
}

void MetronomeDisplay::drawControl(const MetronomeState& state) {
  M5.Display.fillRect(0, 32, M5.Display.width(), 40, TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  if (state.controlMode() == ControlMode::Sound) {
    M5.Display.setTextSize(1);
    M5.Display.setCursor(32, 38);
    M5.Display.printf("A ACCENT  %s", clickSoundName(state.accentClick()));
    M5.Display.setCursor(32, 56);
    M5.Display.printf("B REGULAR %s", clickSoundName(state.regularClick()));
    drawInputStatus(state.controlMode());
    return;
  }

  M5.Display.setTextSize(3);
  M5.Display.setCursor(70, 38);
  if (state.controlMode() == ControlMode::Tempo) {
    M5.Display.printf("%u", state.tempoBpm());
  } else {
    M5.Display.printf("%u", state.clickVolume());
  }

  M5.Display.setTextSize(1);
  M5.Display.setCursor(132, 53);
  M5.Display.print(state.controlMode() == ControlMode::Tempo ? "BPM" : "VOL");
  drawInputStatus(state.controlMode());
}

void MetronomeDisplay::drawInputStatus(ControlMode mode) {
  M5.Display.fillRect(0, STATUS_ROW_Y, M5.Display.width(), STATUS_ROW_HEIGHT,
                      TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, STATUS_ROW_Y + 3);
  if (mode == ControlMode::Sound) {
    M5.Display.print("SOUND   A accent   B regular");
  } else {
    M5.Display.printf("%s   A +   B -",
                      mode == ControlMode::Tempo ? "TEMPO" : "VOLUME");
  }
}

void MetronomeDisplay::drawScreen(const MetronomeState& state) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.print("METRONOME");

  drawControl(state);
  for (uint8_t beat = 0; beat < MetronomeState::BEATS_PER_BAR; beat++) {
    drawBeat(beat, beat == state.currentBeat());
  }
}
