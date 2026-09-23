#include "MetronomeClickSamples.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace {
constexpr uint32_t SAMPLE_RATE_HZ = 16000;
constexpr size_t SAMPLE_COUNT = 2560;  // 160 ms
constexpr float FULL_CYCLE_RADIANS = 6.28318530718f;

int8_t rimBlend[SAMPLE_COUNT];

const PcmS8Sample METRONOME_CLICK_SAMPLE = {
    rimBlend,
    SAMPLE_COUNT,
    SAMPLE_RATE_HZ,
};

uint32_t noiseState = 0x6D2B79F5u;

int8_t nextNoise() {
  noiseState ^= noiseState << 13;
  noiseState ^= noiseState >> 17;
  noiseState ^= noiseState << 5;
  return static_cast<int8_t>(noiseState >> 24);
}

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

void buildMetronomeClickSample() {
  noiseState = 0x6D2B79F5u;
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    const float envelope = squaredEnvelope(index);
    const float low =
        std::sin(FULL_CYCLE_RADIANS * 940.0f * seconds) * 68.0f;
    const float high =
        std::sin(FULL_CYCLE_RADIANS * 2310.0f * seconds) * 46.0f;
    const float strike = static_cast<float>(nextNoise()) * 0.14f;
    rimBlend[index] = clampSample((low + high + strike) * envelope);
  }
}

const PcmS8Sample& metronomeClickSample() {
  return METRONOME_CLICK_SAMPLE;
}

const char* metronomeClickName() {
  return "rim_blend";
}
