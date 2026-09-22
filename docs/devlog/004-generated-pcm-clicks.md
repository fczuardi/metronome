# Generated PCM clicks

## Goal

Replace the functional but tonal buzzer beeps with the generated click pair
selected through the sibling `monophonic-instrument` hardware probe, while
leaving the validated beat clock and catch-up policy unchanged.

## Package boundary

The app now consumes published `fcz2/m5-tone-output@0.1.4` and calls its
`PcmS8Sample`/`playSample()` API. The package owns M5Unified speaker setup,
volume, channel selection, and replacement. The metronome owns its musical
sounds and generates their fixed buffers once during setup.

No PCM generator contract is shared yet. The recipes are copied locally as two
concrete sounds because a broader BFXR-like model has only one experimental
consumer so far.

## Selected pair

Both sounds are mono signed 8-bit PCM at 16 kHz, 2560 samples, and 160 ms:

- regular beat: `low knock`, a 260 Hz sine body with squared decay;
- downbeat: `bright clave`, a 1760 Hz sine body with squared decay.

M5 speaker volume is 128, matching the hardware-selected click-lab pass. The
two fixed buffers use 5120 bytes of RAM.

PCM playback ends on its own, so the app no longer owns tone stop deadlines.
Each beat simply calls `playSample()`. If a delayed loop crosses multiple beat
deadlines, the existing clock still advances arithmetically and triggers only
the resulting current beat; stale clicks are never replayed.

## Verification

```sh
just build
just upload
just monitor
```

On hardware:

1. hear one low knock on beats two through four;
2. hear one bright clave on beat one;
3. confirm visual and audible beat positions agree;
4. confirm every click reports `ok=yes` and ordinary beats report `elapsed=1`;
5. mash both buttons and confirm no missed, doubled, queued, or drifting clicks;
6. leave the device running to verify stable cadence and phase.

Serial timestamps remain event-loop observations rather than measurements of
physical acoustic onset.

## Hardware result

The M5StickC Plus2 stress run observed beats from `26542` through `49542 ms`.
Every interval remained exactly 500 ms on the established `...542` phase,
including a sustained section of dense A/B press and release traffic. Every
beat reported `elapsed=1`, and every low-knock and bright-clave playback request
reported `ok=yes`.

Beat order and sound selection remained correct throughout: bright clave only
on beat one and low knock on beats two through four. The log contains no stale
catch-up burst, missed beat, doubled beat, rejected sample, or cumulative
phase drift. This validates the published 0.1.4 PCM path under the same
interaction pressure previously used for tone playback.

As before, this software log establishes request cadence and success rather
than measuring physical acoustic onset. The two sounds themselves were already
selected by listening to the identical recipes in the sibling click probe.
