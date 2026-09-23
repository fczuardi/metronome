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

void BeatClock::setIntervalMs(uint32_t nowMs, uint32_t intervalMs) {
  if (intervalMs == 0 || intervalMs_ == 0) return;

  if (!started_) {
    intervalMs_ = intervalMs;
    return;
  }

  const int32_t signedRemaining =
      static_cast<int32_t>(nextBeatAtMs_ - nowMs);
  const uint32_t remainingMs = signedRemaining > 0 ? signedRemaining : 0;
  const uint32_t scaledRemainingMs = static_cast<uint32_t>(
      (static_cast<uint64_t>(remainingMs) * intervalMs + intervalMs_ / 2) /
      intervalMs_);

  intervalMs_ = intervalMs;
  nextBeatAtMs_ = nowMs + scaledRemainingMs;
}
