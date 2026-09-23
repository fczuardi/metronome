# Application boundaries

## Goal

Refactor the validated metronome without changing its behavior, giving display,
input, state, audio, timing, and sound generation explicit local ownership
before adding more modes or transport behavior.

## Boundaries

`main.cpp` had reached 294 lines and directly owned four different concerns.
The refactor keeps orchestration and serial reporting there while extracting:

- `ControlSurface`: converts M5 button state into `Increase`, `Decrease`, or
  `NextMode`; its rearm rule guarantees chord suppression;
- `MetronomeState`: owns tempo, volume, beat position, mode, and both sound
  selections, including bounds and transitions;
- `MetronomeDisplay`: owns coordinates, labels, mode-specific values, and beat
  indicators;
- `MetronomeAudio`: owns `M5BuzzerToneOutput`, master volume, channel-1 silent
  keep-alive, and one-shot dispatch.

The existing `BeatClock` and `MetronomeClickSamples` boundaries remain intact.
All storage stays fixed-size and no event path allocates dynamically.

The modules remain application-local. In particular, silent keep-alive and the
three-command control surface are discoveries from one device/application;
they are not promoted to shared packages without another consumer.

## Event ordering

The loop retains the validated order:

1. update M5 input state;
2. advance any due beat under the old tempo;
3. decode at most one control gesture;
4. apply its mode-specific state transition;
5. yield briefly.

This preserves the rule that a tempo edit at a deadline cannot silently skip
the due beat. `main.cpp` remains responsible for coordinating state changes
with side effects: clock interval changes, display updates, audio gain, and
serial facts.

## Verification

```sh
just build
just upload
just monitor
```

The refactor build uses 16.4% RAM and 39.2% flash. On hardware, compare against
the preceding slice:

1. boot at 120 BPM, volume 128, plain-1600 accent, and mid-tick regular sound;
2. confirm the silent keep-alive reports `ok=yes` and no output-start transient
   returns;
3. confirm the mode cycle remains Tempo -> Volume -> Sound -> Tempo;
4. confirm A increases and B decreases tempo and volume by the same steps;
5. confirm A selects accents and B selects regular sounds, with all ten entries
   in the same order;
6. confirm every chord suppresses individual button commands until both
   buttons are released;
7. change tempo and confirm phase continuity, correct beat order, and
   `elapsed=1` during ordinary operation;
8. confirm display text, audible output, and serial records remain aligned.

This slice is successful only if hardware behavior is equivalent; cleaner
source layout alone is not sufficient validation.
