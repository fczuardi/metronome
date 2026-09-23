#pragma once

#include <cstdint>

#include "MetronomeClickSamples.h"

enum class ControlMode : uint8_t {
  Tempo,
  Volume,
  Sound,
};

class MetronomeState {
 public:
  static constexpr uint16_t MIN_TEMPO_BPM = 30;
  static constexpr uint16_t MAX_TEMPO_BPM = 300;
  static constexpr uint8_t VOLUME_STEP = 8;
  static constexpr uint8_t BEATS_PER_BAR = 4;

  uint16_t tempoBpm() const;
  uint32_t beatIntervalMs() const;
  uint8_t clickVolume() const;
  uint8_t currentBeat() const;
  ControlMode controlMode() const;
  const char* controlModeName() const;
  ClickSoundId accentClick() const;
  ClickSoundId regularClick() const;

  void cycleControlMode();
  bool adjustTempo(int8_t direction);
  bool adjustVolume(int8_t direction);
  void selectNextSound(bool accent);
  void advanceBeat(uint32_t elapsedBeats);

 private:
  uint16_t tempoBpm_ = 120;
  uint8_t clickVolume_ = 128;
  uint8_t currentBeat_ = 0;
  ControlMode controlMode_ = ControlMode::Tempo;
  ClickSoundId accentClick_ = ClickSoundId::Plain1600;
  ClickSoundId regularClick_ = ClickSoundId::MidTick;
};
