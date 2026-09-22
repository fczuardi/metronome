#pragma once

#include <cstddef>
#include <cstdint>

#include "PcmS8Sample.h"

enum class ClickSoundId : uint8_t {
  DryImpulse,
  RimBlend,
  DownSweep,
  LowKnock,
  Count,
};

void buildMetronomeClickSamples();
size_t clickSoundCount();
const char* clickSoundName(ClickSoundId id);
const char* clickSoundLogName(ClickSoundId id);
const PcmS8Sample& clickSoundSample(ClickSoundId id);
