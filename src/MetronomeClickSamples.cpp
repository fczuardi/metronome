#include "MetronomeClickSamples.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace {
constexpr uint32_t SAMPLE_RATE_HZ = 16000;
constexpr size_t SAMPLE_COUNT = 2560;  // 160 ms
constexpr float FULL_CYCLE_RADIANS = 6.28318530718f;

int8_t lowKnock[SAMPLE_COUNT];
int8_t brightClave[SAMPLE_COUNT];

const PcmS8Sample REGULAR_BEAT_SAMPLE = {
    lowKnock,
    SAMPLE_COUNT,
    SAMPLE_RATE_HZ,
};
const PcmS8Sample DOWNBEAT_SAMPLE = {
    brightClave,
    SAMPLE_COUNT,
    SAMPLE_RATE_HZ,
};

float squaredEnvelope(size_t index) {
  const float remaining = 1.0f - static_cast<float>(index) / SAMPLE_COUNT;
  return remaining * remaining;
}

int8_t clampSample(float value) {
  if (value > 127.0f) return 127;
  if (value < -128.0f) return -128;
  return static_cast<int8_t>(value);
}
}  // namespace

void buildMetronomeClickSamples() {
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    const float envelope = squaredEnvelope(index);

    const float lowBody =
        std::sin(FULL_CYCLE_RADIANS * 260.0f * seconds) * 112.0f;
    lowKnock[index] = clampSample(lowBody * envelope);

    const float claveBody =
        std::sin(FULL_CYCLE_RADIANS * 1760.0f * seconds) * 112.0f;
    brightClave[index] = clampSample(claveBody * envelope);
  }
}

const PcmS8Sample& regularBeatSample() {
  return REGULAR_BEAT_SAMPLE;
}

const PcmS8Sample& downbeatSample() {
  return DOWNBEAT_SAMPLE;
}
