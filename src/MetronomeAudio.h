#pragma once

#include <cstdint>

#include "AudioIdlePolicy.h"
#include "M5BuzzerToneOutput.h"
#include "PcmS8Sample.h"

class MetronomeAudio {
 public:
  bool begin(uint8_t volume);
  void setVolume(uint8_t volume);
  bool play(const PcmS8Sample& sample);

 private:
  M5BuzzerToneOutput output_{AudioIdlePolicy::KeepAlive};
};
