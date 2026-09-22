#pragma once

#include <stdint.h>

class BeatClock {
 public:
  explicit BeatClock(uint32_t intervalMs) : intervalMs_(intervalMs) {}

  void begin(uint32_t nowMs);
  uint32_t elapsedBeats(uint32_t nowMs);

 private:
  uint32_t intervalMs_;
  uint32_t nextBeatAtMs_ = 0;
  bool started_ = false;
};

