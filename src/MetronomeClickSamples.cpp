#include "MetronomeClickSamples.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace {
constexpr uint32_t SAMPLE_RATE_HZ = 16000;
constexpr size_t SAMPLE_COUNT = 2560;  // 160 ms
constexpr float FULL_CYCLE_RADIANS = 6.28318530718f;

int8_t dryImpulse[SAMPLE_COUNT];
int8_t rimBlend[SAMPLE_COUNT];
int8_t downSweep[SAMPLE_COUNT];
int8_t lowKnock[SAMPLE_COUNT];

const PcmS8Sample SAMPLES[] = {
    {dryImpulse, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {rimBlend, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {downSweep, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {lowKnock, SAMPLE_COUNT, SAMPLE_RATE_HZ},
};

const char* NAMES[] = {
    "dry impulse", "rim blend", "down sweep", "low knock"};
const char* LOG_NAMES[] = {
    "dry_impulse", "rim_blend", "down_sweep", "low_knock"};

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

void buildMetronomeClickSamples() {
  noiseState = 0x6D2B79F5u;
  float sweepPhase = 0.0f;

  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    const float position = static_cast<float>(index) / SAMPLE_COUNT;
    const float envelope = squaredEnvelope(index);

    dryImpulse[index] = 0;

    const float rimLow =
        std::sin(FULL_CYCLE_RADIANS * 940.0f * seconds) * 68.0f;
    const float rimHigh =
        std::sin(FULL_CYCLE_RADIANS * 2310.0f * seconds) * 46.0f;
    const float rimStrike = static_cast<float>(nextNoise()) * 0.14f;
    rimBlend[index] =
        clampSample((rimLow + rimHigh + rimStrike) * envelope);

    const float sweepFrequencyHz = 1900.0f - 1500.0f * position;
    sweepPhase += FULL_CYCLE_RADIANS * sweepFrequencyHz / SAMPLE_RATE_HZ;
    downSweep[index] =
        clampSample(std::sin(sweepPhase) * 104.0f * envelope);

    lowKnock[index] = clampSample(
        std::sin(FULL_CYCLE_RADIANS * 260.0f * seconds) * 112.0f * envelope);
  }

  constexpr size_t IMPULSE_SPACING_SAMPLES = 160;  // 10 ms
  for (size_t index = 0; index < SAMPLE_COUNT;
       index += IMPULSE_SPACING_SAMPLES) {
    const float envelope = squaredEnvelope(index);
    dryImpulse[index] = clampSample(127.0f * envelope);
    dryImpulse[index + 1] = clampSample(-128.0f * envelope);
  }
}

size_t clickSoundCount() {
  return static_cast<size_t>(ClickSoundId::Count);
}

const char* clickSoundName(ClickSoundId id) {
  const size_t index = static_cast<size_t>(id);
  return index < clickSoundCount() ? NAMES[index] : "unknown";
}

const char* clickSoundLogName(ClickSoundId id) {
  const size_t index = static_cast<size_t>(id);
  return index < clickSoundCount() ? LOG_NAMES[index] : "unknown";
}

const PcmS8Sample& clickSoundSample(ClickSoundId id) {
  const size_t index = static_cast<size_t>(id);
  return SAMPLES[index < clickSoundCount() ? index : 0];
}
