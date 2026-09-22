# Click-pair audition

## Goal

Compare strong generated recipes in the real metronome cadence,
both as the downbeat accent and as the repeating regular beat. This tests pairs
rather than judging either role in isolation.

## Interaction

Button A cycles the accent and Button B independently cycles the regular beat
through:

1. dry impulse;
2. rim blend;
3. down sweep;
4. low knock.

The initial pair is `rim blend` for beat 1 and `low knock` for beats 2–4, based
on the hardware comparison. A 12 ms smooth-attack variant of low knock was
also compared on-device, but its difference was barely audible, so the simpler
original recipe remains. Each selection appears on screen and is reported as
`accent_sound: selected=...` or
`regular_sound: selected=...`. Every `click:` record names the sound actually
requested for that beat.

All four 160 ms buffers are built once during setup. Playback performs only a
bounded lookup and passes immutable PCM to `m5-tone-output`; it does not
allocate or regenerate sound in the beat event path. The audition firmware's
fixed PCM storage is 10240
bytes.

## Verification

```sh
just build
just upload
just monitor
```

On hardware:

1. confirm the initial rim-blend/low-knock pair;
2. press A four times and confirm only beat 1 cycles and wraps;
3. press B four times and confirm only beats 2–4 cycle and wrap;
4. try promising pairs, including matching accent and regular sounds;
5. change either role immediately before and during a beat and confirm there
   are no doubled, missed, or queued clicks;
6. confirm playback reports `ok=yes`, normal beats report `elapsed=1`, and the
   500 ms phase remains stable.

This remains an app-local listening experiment. It does not promote the recipe
catalog or synthesis parameters into the audio package.
