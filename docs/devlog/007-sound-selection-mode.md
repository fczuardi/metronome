# Sound-selection mode

## Goal

Extend the validated two-button mode cycle with independent accent and regular
click selection, now that silent keep-alive prevents the speaker-start
transient from masquerading as part of a sound.

## Interaction

The A+B cycle is now:

```text
Tempo -> Volume -> Sound -> Tempo
```

In Sound mode, releasing A advances the beat-1 accent and releasing B advances
the regular beats. The A+B chord retains priority and returns to Tempo without
changing either selection. The final ten-sound catalog starts on plain 1600
for the accent and mid tick for regular beats.

The screen shows both assignments simultaneously. The initial catalog had four
app-local candidates:

1. dry impulse;
2. rim blend;
3. down sweep;
4. low knock.

The first clean audition eliminated low knock: without the electrical start
transient it stopped being useful too early, becoming barely audible around
volume 144. Dry impulse, rim blend, and down sweep survived, with the latter
two the most broadly usable. Their stronger high-frequency content suggests a
better match for the M5StickC Plus2 transducer.

The catalog was therefore expanded to eleven PCM recipes biased toward that
range:

1. dry impulse: a decaying 10 ms bipolar impulse train;
2. rim blend: 940/2310 Hz bodies plus a noisy strike;
3. down sweep: 1900 to 400 Hz;
4. mid tick: a fast-decaying 1400 Hz sine;
5. high tick: a fast-decaying 2600 Hz sine;
6. dual tone: a 1320/2190 Hz blend;
7. metallic: an inharmonic 1170/1810/2770 Hz cluster;
8. up sweep: 700 to 2900 Hz;
9. bright noise: fast-decaying high-passed deterministic noise;
10. plain 1600: a deliberately simple decaying sine;
11. plain 2400: the same simple recipe at a higher frequency.

The plain tones provide useful controls: if they outperform more elaborate
recipes, spectral placement matters more than synthesis complexity on this
transducer. All options are signed 8-bit mono PCM at 16 kHz and 160 ms; no tone
renderer or electrical start transient participates in the comparison.

## Expanded-catalog hardware result

All eight new recipes sounded distinct and useful on hardware. In particular,
dual tone and metallic were cleaner, improved versions of the same broad idea
as rim blend. Rim blend was therefore removed rather than retaining a weaker
near-duplicate.

The resulting ten-sound library is:

1. dry impulse;
2. down sweep;
3. mid tick;
4. high tick;
5. dual tone;
6. metallic;
7. up sweep;
8. bright noise;
9. plain 1600;
10. plain 2400.

The hardware-selected default pair uses plain 1600 for the beat-1 accent and
mid tick for regular beats. Both remain independently selectable in Sound
mode.

Every candidate is generated once into fixed storage during setup. Selection
performs a bounded lookup, and the silent channel-1 keep-alive from the
preceding slice remains active.

The first clean audition found rim blend and down sweep the most useful,
probably because their higher-frequency content falls within the tiny
transducer's stronger range. Even those recipes lost their character below
roughly volume 48; 64 was usable, while 128 and above were clearer.

Part of that degradation came from the temporary PCM-domain gain experiment.
It squared the 0–255 control and quantized the result back to signed 8-bit
samples. At volume 48, a source peak of 112 became only about four integer
levels, destroying waveform detail before the backend saw it. That workaround
was useful to separate PCM from the electrical start transient, but it is not
a good final volume implementation.

With silent keep-alive preventing output wake/sleep transitions, volume now
returns to the package/M5Unified master-gain path and the original 8-bit PCM is
played unchanged. M5Unified applies its gain during mixing, retaining more
precision than rewriting the source buffer. This also removes the 2560-byte
scratch buffer and lets zero volume mute normally without skipping beat
events.

## Verification

```sh
just build
just upload
just monitor
```

On hardware:

1. chord from Tempo through Volume into Sound and confirm no stray value or
   sound changes;
2. press A ten times and confirm only the accent cycles and wraps;
3. press B ten times and confirm only the regular click cycles and wraps;
4. confirm the screen assignments match the `click:` records;
5. compare the finalists at volumes 32, 48, 64, 128, and above, checking whether
   master gain preserves more character than PCM scaling;
6. mash overlapping A/B gestures and confirm each produces at most one command
   while beat timing remains stable.

The catalog remains application-owned. The experiment selects musical sounds;
it does not yet establish a reusable synthesis-recipe API.
