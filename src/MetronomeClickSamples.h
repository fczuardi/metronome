#pragma once

#include <cstddef>
#include <cstdint>

#include "PcmS8Sample.h"

enum class ClickSoundId : uint8_t {
  DryImpulse,
  DownSweep,
  MidTick,
  HighTick,
  DualTone,
  Metallic,
  UpSweep,
  BrightNoise,
  Plain1600,
  Plain2400,
  Count,
};

void buildMetronomeClickSamples();
size_t clickSoundCount();
const char* clickSoundName(ClickSoundId id);
const char* clickSoundLogName(ClickSoundId id);
const PcmS8Sample& clickSoundSample(ClickSoundId id);
