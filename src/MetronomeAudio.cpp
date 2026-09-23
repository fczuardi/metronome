#include "MetronomeAudio.h"

#include <Arduino.h>
#include <M5Unified.h>

bool MetronomeAudio::begin(uint8_t volume) {
  output_.begin();
  output_.setVolume(volume);
  return M5.Speaker.playRaw(
      keepAliveSilence_, KEEP_ALIVE_SAMPLE_COUNT, SAMPLE_RATE_HZ, false,
      UINT32_MAX, KEEP_ALIVE_CHANNEL, true);
}

void MetronomeAudio::setVolume(uint8_t volume) {
  output_.setVolume(volume);
}

bool MetronomeAudio::play(const PcmS8Sample& sample) {
  return output_.playSample(sample);
}
