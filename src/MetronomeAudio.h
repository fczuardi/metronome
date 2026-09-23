#pragma once

#include <cstddef>
#include <cstdint>

#include "M5BuzzerToneOutput.h"
#include "PcmS8Sample.h"

class MetronomeAudio {
 public:
  static constexpr size_t KEEP_ALIVE_SAMPLE_COUNT = 256;
  static constexpr int KEEP_ALIVE_CHANNEL = 1;

  bool begin(uint8_t volume);
  void setVolume(uint8_t volume);
  bool play(const PcmS8Sample& sample);

 private:
  static constexpr uint32_t SAMPLE_RATE_HZ = 16000;

  M5BuzzerToneOutput output_;
  int8_t keepAliveSilence_[KEEP_ALIVE_SAMPLE_COUNT] = {};
};
