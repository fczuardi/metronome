# PlatformIO baseline

## Goal

Establish a buildable M5StickC Plus2 application and verify the hardware edge
before introducing musical-time semantics.

The validation target is deliberately narrow: initialize M5Unified, render a
legible landscape screen, and report press and release edges for Buttons A and
B through both the display and Serial.

## Repository role

This repository owns the exploratory metronome application. It is separate
from `embedded-music-experiments` because the umbrella's showcases should be
thin compositions of proven packages, while the first metronome slices will
contain substantial timing, interaction, and display policy.

It is also separate from the audio-output repositories. A click backend may
consume an existing buzzer component later, but tempo, meter, transport, and
click scheduling are not audio-backend responsibilities.

## Build design

The PlatformIO environment uses the platform's `m5stick-c` board definition
with the Arduino framework's `m5stack_stickc_plus2` variant and M5Unified
0.2.17. The current pioarduino board registry does not expose a separate
`m5stick-cplus2` board ID. Its `core_dir` points at the workspace's shared
`../.platformio-home` cache so sibling experiments reuse the installed
toolchain and packages.

The firmware explicitly enables the internal speaker, disables the unused
microphone, and supplies the Plus2 fallback board identifier. The buzzer is not
activated in this slice.

## Timing direction

The future metronome will compare its timing model with the Calculator drum
sequencer, using Standard MIDI Files and MIDI realtime transport as vocabulary
references. The design keeps these concepts distinct:

- musical pulse or position;
- real monotonic time and deadlines;
- tempo conversion;
- meter, beat, and downbeat projection;
- transport state;
- scheduled application events;
- audio and display output policy.

No shared transport, clock, scheduler, or event contract is introduced by this
baseline.

## Verification

```sh
just build
just upload
just monitor
```

On the M5StickC Plus2:

1. confirm that the display is landscape and shows `METRONOME`;
2. press and release Button A and confirm both edges appear on screen and in
   Serial;
3. repeat for Button B;
4. confirm that the passive buzzer remains silent.

## Hardware result

Validated on the M5StickC Plus2. The landscape screen displayed the baseline
text correctly, and Buttons A and B each produced observable press and release
edges.

The current `drawScreen()` implementation redraws the entire display for every
button edge. That is acceptable for this silent initialization slice. Once a
clock and buzzer establish timing-sensitive work, later slices should measure
whether full redraws disturb scheduling and move to partial status updates if
needed rather than optimizing the display preemptively.
