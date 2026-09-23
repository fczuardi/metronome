#include "MetronomeAudio.h"

bool MetronomeAudio::begin(uint8_t volume) {
  output_.setVolume(volume);
  return output_.begin();
}

void MetronomeAudio::setVolume(uint8_t volume) {
  output_.setVolume(volume);
}

bool MetronomeAudio::play(const PcmS8Sample& sample) {
  return output_.playSample(sample);
}
