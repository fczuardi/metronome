#include "ControlSurface.h"

#include <M5Unified.h>

ControlCommand ControlSurface::poll() {
  if (!armed_) {
    if (!M5.BtnA.isPressed() && !M5.BtnB.isPressed()) armed_ = true;
    return ControlCommand::None;
  }

  if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
    armed_ = false;
    return ControlCommand::NextMode;
  }

  if (M5.BtnA.wasReleased() && !M5.BtnB.isPressed()) {
    armed_ = false;
    return ControlCommand::Increase;
  }

  if (M5.BtnB.wasReleased() && !M5.BtnA.isPressed()) {
    armed_ = false;
    return ControlCommand::Decrease;
  }

  return ControlCommand::None;
}
