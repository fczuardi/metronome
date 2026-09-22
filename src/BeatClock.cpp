#include "BeatClock.h"

void BeatClock::begin(uint32_t nowMs) {
  nextBeatAtMs_ = nowMs + intervalMs_;
  started_ = true;
}

uint32_t BeatClock::elapsedBeats(uint32_t nowMs) {
  if (!started_ || intervalMs_ == 0 ||
      static_cast<int32_t>(nowMs - nextBeatAtMs_) < 0) {
    return 0;
  }

  const uint32_t beatCount = (nowMs - nextBeatAtMs_) / intervalMs_ + 1;
  nextBeatAtMs_ += beatCount * intervalMs_;
  return beatCount;
}

