#include "MetronomeClickSamples.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace {
constexpr uint32_t SAMPLE_RATE_HZ = 16000;
constexpr size_t SAMPLE_COUNT = 2560;  // 160 ms
constexpr float FULL_CYCLE_RADIANS = 6.28318530718f;

int8_t dryImpulse[SAMPLE_COUNT];
int8_t downSweep[SAMPLE_COUNT];
int8_t midTick[SAMPLE_COUNT];
int8_t highTick[SAMPLE_COUNT];
int8_t dualTone[SAMPLE_COUNT];
int8_t metallic[SAMPLE_COUNT];
int8_t upSweep[SAMPLE_COUNT];
int8_t brightNoise[SAMPLE_COUNT];
int8_t plain1600[SAMPLE_COUNT];
int8_t plain2400[SAMPLE_COUNT];

const PcmS8Sample SAMPLES[] = {
    {dryImpulse, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {downSweep, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {midTick, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {highTick, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {dualTone, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {metallic, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {upSweep, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {brightNoise, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {plain1600, SAMPLE_COUNT, SAMPLE_RATE_HZ},
    {plain2400, SAMPLE_COUNT, SAMPLE_RATE_HZ},
};

const char* NAMES[] = {
    "dry impulse", "down sweep", "mid tick",   "high tick",
    "dual tone",   "metallic",   "up sweep",
    "bright noise", "plain 1600", "plain 2400",
};

const char* LOG_NAMES[] = {
    "dry_impulse", "down_sweep", "mid_tick",   "high_tick",
    "dual_tone",   "metallic",   "up_sweep",
    "bright_noise", "plain_1600", "plain_2400",
};

uint32_t noiseState = 0x6D2B79F5u;

int8_t nextNoise() {
  noiseState ^= noiseState << 13;
  noiseState ^= noiseState >> 17;
  noiseState ^= noiseState << 5;
  return static_cast<int8_t>(noiseState >> 24);
}

float positionAt(size_t index) {
  return static_cast<float>(index) / SAMPLE_COUNT;
}

float squaredEnvelope(size_t index) {
  const float remaining = 1.0f - positionAt(index);
  return remaining * remaining;
}

float fastEnvelope(size_t index) {
  const float squared = squaredEnvelope(index);
  return squared * squared;
}

int8_t clampSample(float value) {
  if (value > 127.0f) return 127;
  if (value < -128.0f) return -128;
  return static_cast<int8_t>(value);
}

float sineAt(float frequencyHz, float seconds) {
  return std::sin(FULL_CYCLE_RADIANS * frequencyHz * seconds);
}

void buildDryImpulse() {
  for (size_t index = 0; index < SAMPLE_COUNT; index++) dryImpulse[index] = 0;

  constexpr size_t IMPULSE_SPACING_SAMPLES = 160;  // 10 ms
  for (size_t index = 0; index < SAMPLE_COUNT;
       index += IMPULSE_SPACING_SAMPLES) {
    const float envelope = squaredEnvelope(index);
    dryImpulse[index] = clampSample(127.0f * envelope);
    dryImpulse[index + 1] = clampSample(-128.0f * envelope);
  }
}

void buildSweep(int8_t* target, float startHz, float endHz) {
  float phase = 0.0f;
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float frequencyHz =
        startHz + (endHz - startHz) * positionAt(index);
    phase += FULL_CYCLE_RADIANS * frequencyHz / SAMPLE_RATE_HZ;
    target[index] =
        clampSample(std::sin(phase) * 104.0f * squaredEnvelope(index));
  }
}

void buildTick(int8_t* target, float frequencyHz) {
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    target[index] = clampSample(
        sineAt(frequencyHz, seconds) * 118.0f * fastEnvelope(index));
  }
}

void buildDualTone() {
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    const float body = sineAt(1320.0f, seconds) * 68.0f +
                       sineAt(2190.0f, seconds) * 54.0f;
    dualTone[index] = clampSample(body * squaredEnvelope(index));
  }
}

void buildMetallic() {
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    const float body = sineAt(1170.0f, seconds) * 48.0f +
                       sineAt(1810.0f, seconds) * 42.0f +
                       sineAt(2770.0f, seconds) * 34.0f;
    metallic[index] = clampSample(body * squaredEnvelope(index));
  }
}

void buildBrightNoise() {
  int8_t previous = nextNoise();
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const int8_t current = nextNoise();
    const float highPassed = static_cast<float>(current - previous) * 0.42f;
    brightNoise[index] = clampSample(highPassed * fastEnvelope(index));
    previous = current;
  }
}

void buildPlainTone(int8_t* target, float frequencyHz) {
  for (size_t index = 0; index < SAMPLE_COUNT; index++) {
    const float seconds = static_cast<float>(index) / SAMPLE_RATE_HZ;
    target[index] = clampSample(
        sineAt(frequencyHz, seconds) * 104.0f * squaredEnvelope(index));
  }
}
}  // namespace

void buildMetronomeClickSamples() {
  noiseState = 0x6D2B79F5u;
  buildDryImpulse();
  buildSweep(downSweep, 1900.0f, 400.0f);
  buildTick(midTick, 1400.0f);
  buildTick(highTick, 2600.0f);
  buildDualTone();
  buildMetallic();
  buildSweep(upSweep, 700.0f, 2900.0f);
  buildBrightNoise();
  buildPlainTone(plain1600, 1600.0f);
  buildPlainTone(plain2400, 2400.0f);
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
