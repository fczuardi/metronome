#pragma once

#include <cstdint>

enum class ControlCommand : uint8_t {
  None,
  Increase,
  Decrease,
  NextMode,
};

class ControlSurface {
 public:
  ControlCommand poll();

 private:
  bool armed_ = true;
};
