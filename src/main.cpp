#include <Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;

void drawScreen(const char* status) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.println("METRONOME");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, 40);
  M5.Display.println("M5StickC Plus2 baseline");
  M5.Display.setCursor(8, 62);
  M5.Display.println("Buttons A/B ready");
  M5.Display.setCursor(8, 78);
  M5.Display.println("Clock and buzzer deferred");

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(8, 108);
  M5.Display.printf("Input: %s", status);
}

void reportButtons() {
  if (M5.BtnA.wasPressed()) {
    Serial.println("button: name=a action=pressed");
    drawScreen("A pressed");
  }
  if (M5.BtnA.wasReleased()) {
    Serial.println("button: name=a action=released");
    drawScreen("A released");
  }
  if (M5.BtnB.wasPressed()) {
    Serial.println("button: name=b action=pressed");
    drawScreen("B pressed");
  }
  if (M5.BtnB.wasReleased()) {
    Serial.println("button: name=b action=released");
    drawScreen("B released");
  }
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
  drawScreen("ready");
  Serial.println("metronome: platformio_baseline=ready board=m5stick-cplus2");
}

void loop() {
  M5.update();
  reportButtons();
  delay(1);
}

