#include "MetronomeState.h"

#include <algorithm>
#include <cstddef>

uint16_t MetronomeState::tempoBpm() const {
  return tempoBpm_;
}

uint32_t MetronomeState::beatIntervalMs() const {
  return 60000UL / tempoBpm_;
}

uint8_t MetronomeState::clickVolume() const {
  return clickVolume_;
}

uint8_t MetronomeState::currentBeat() const {
  return currentBeat_;
}

ControlMode MetronomeState::controlMode() const {
  return controlMode_;
}

const char* MetronomeState::controlModeName() const {
  switch (controlMode_) {
    case ControlMode::Tempo:
      return "tempo";
    case ControlMode::Volume:
      return "volume";
    case ControlMode::Sound:
      return "sound";
  }
  return "unknown";
}

ClickSoundId MetronomeState::accentClick() const {
  return accentClick_;
}

ClickSoundId MetronomeState::regularClick() const {
  return regularClick_;
}

void MetronomeState::cycleControlMode() {
  switch (controlMode_) {
    case ControlMode::Tempo:
      controlMode_ = ControlMode::Volume;
      break;
    case ControlMode::Volume:
      controlMode_ = ControlMode::Sound;
      break;
    case ControlMode::Sound:
      controlMode_ = ControlMode::Tempo;
      break;
  }
}

bool MetronomeState::adjustTempo(int8_t direction) {
  const int16_t candidate = static_cast<int16_t>(tempoBpm_) + direction;
  const uint16_t bounded = static_cast<uint16_t>(
      std::max<int16_t>(MIN_TEMPO_BPM,
                        std::min<int16_t>(MAX_TEMPO_BPM, candidate)));
  if (bounded == tempoBpm_) return false;
  tempoBpm_ = bounded;
  return true;
}

bool MetronomeState::adjustVolume(int8_t direction) {
  const int16_t candidate =
      static_cast<int16_t>(clickVolume_) + direction * VOLUME_STEP;
  const uint8_t bounded = static_cast<uint8_t>(
      std::max<int16_t>(0, std::min<int16_t>(255, candidate)));
  if (bounded == clickVolume_) return false;
  clickVolume_ = bounded;
  return true;
}

void MetronomeState::selectNextSound(bool accent) {
  ClickSoundId& selection = accent ? accentClick_ : regularClick_;
  const size_t nextIndex =
      (static_cast<size_t>(selection) + 1) % clickSoundCount();
  selection = static_cast<ClickSoundId>(nextIndex);
}

void MetronomeState::advanceBeat(uint32_t elapsedBeats) {
  currentBeat_ = (currentBeat_ + elapsedBeats) % BEATS_PER_BAR;
}
