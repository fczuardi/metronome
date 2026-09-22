# Audible accented clicks

## Goal

Add physical buzzer output without changing the validated beat progression.
Every current beat should produce one short click, with a clearly distinct
downbeat, while delayed loops must never replay stale clicks.

## Output boundary

The app consumes the published `m5-tone-output` 0.1.3 package and uses
`M5BuzzerToneOutput` for Plus2 GPIO, M5Unified speaker, sample-rate, waveform,
and volume setup. It deliberately does not instantiate `MonophonicInstrument`
or translate clicks into MIDI note events.

The existing package is note-oriented at its abstract `VoiceOutput` boundary,
but its concrete buzzer backend also exposes lower-level `startTone()` and
`stop()` operations. The metronome uses those operations as hardware plumbing.
This second consumer can reveal whether a future trigger or pulse-output
contract is warranted; it does not introduce one yet.

## Click policy

The initial sound pair is:

| Beat | Frequency | Duration |
| --- | ---: | ---: |
| Downbeat | 2000 Hz | 45 ms |
| Regular | 1200 Hz | 25 ms |

The application owns the short articulation deadline. Starting a beat turns
the tone on and records a stop deadline; the normal event loop turns it off.
Click duration therefore remains independent from the 500 ms musical beat
interval.

If the event loop misses multiple beat deadlines, `BeatClock` advances across
the complete elapsed count, while the app triggers only the resulting current
beat. It never emits an audio burst representing deadlines already in the
past. The click log remains separate from the beat log so onset requests and
musical progression are independently observable.

## Verification

```sh
just build
just upload
just monitor
```

On the M5StickC Plus2:

1. confirm one audible click accompanies every visible beat;
2. confirm beat one is noticeably higher and longer than beats two through
   four;
3. listen for a stable cadence without clicks bleeding into one another;
4. mash A and B and confirm neither visual feedback nor Serial activity causes
   a missed or doubled click;
5. confirm every click log reports `ok=yes` and every beat log reports
   `elapsed=1` during ordinary operation;
6. leave the device running and listen for phase changes between sound and the
   highlighted beat.

Serial timestamps still measure event-loop observation, not physical buzzer
onset. Precise audio jitter and long-term oscillator accuracy require a
microphone recording or GPIO measurement in a later diagnostic pass.

## Hardware result

The first idle run on the M5StickC Plus2 logged beat observations from `1001 ms`
through `41501 ms`. All 82 beat intervals were exactly 500 ms, every beat
reported `elapsed=1`, and every corresponding click request reported `ok=yes`.
The 2000 Hz, 45 ms accent appeared on each beat one, while beats two through
four consistently requested the 1200 Hz, 25 ms regular click.

This confirms that initializing and driving `M5BuzzerToneOutput` did not disturb
the previously validated software deadline phase. Serial proves successful
onset requests, not physical click duration or acoustic onset. Perceptual
confirmation of one click per beat, downbeat distinction, and button-stress
behavior remains pending.

A subsequent button-smash pass interleaved hundreds of A/B edges with audible
playback through `106001 ms`. Every beat still reported `elapsed=1`, every click
request returned `ok=yes`, and accent order remained correct. Two observations
at `87002` and `96502 ms` arrived 1 ms after the established `...001` phase; the
following beat in each case returned to that phase. Button input therefore
introduced no software-side missed or doubled click and no cumulative drift.

Listening confirmed exactly one clean physical click per visible beat and a
clearly different downbeat accent. The click onset sounded synchronized to the
display in an informal listening test. The current saw-wave tones are
functional but uncomfortably high and beep-like; timbre tuning remains a
separate hardware-listening decision rather than a timing failure.

## Timbre follow-up

Earlier Plus2 buzzer probes found that low requested pitches can lose their
fundamental, become barely audible, or turn noisy. The buzzer radiated more
efficiently around C6-C7, and M5Unified volume `64..128` was the useful range;
higher gain distorted while lower gain became hard to recognize. The proposed
800 Hz regular click was therefore not used.

The comparison instead stays in the proven frequency and gain region:

| Beat | Frequency | Duration | Volume |
| --- | ---: | ---: | ---: |
| Downbeat | 1568 Hz (about G6) | 30 ms | 96 |
| Regular | 1047 Hz (about C6) | 20 ms | 96 |

This changes pitch, duration, and gain together because the practical question
is whether a short metronome click becomes more comfortable while remaining
audible and distinct. The original 2000/1200 Hz pair remains documented above
as the timing-validation baseline.

The lower pair was immediately more comfortable but still recognizably beepy.
The next comparison changes only duration: the regular click shrinks from 20 to
6 ms, and the downbeat from 30 to 10 ms. At the requested frequencies these
contain roughly six and sixteen waveform cycles, respectively, reducing the
time available for the ear to identify a sustained pitch. Frequency, volume,
and `saw32` waveform remain fixed so this pass isolates articulation length.

Hardware result: shortening the clicks did not materially reduce their beepy
character. This indicates that the stable pitch, rather than duration alone,
dominates the perceived timbre.

## Two-stage descending chirp

The next comparison replaces each stable tone with a short descending contour
while retaining the 6 ms regular and 10 ms downbeat durations. The regular
click begins at 1397 Hz, falls to 1047 Hz after 2 ms, and then stops. The
downbeat begins at 1865 Hz, falls to 1397 Hz after 3 ms, and then stops.

This stays in the buzzer's proven audible range, avoids returning to the
original 2 kHz accent, and changes only the pitch contour. A small non-blocking
click state machine schedules the tail transition and stop alongside the beat
clock and button polling.

Hardware result: the descending contour did not materially reduce the beepy
perception at those frequencies.

## Lower-frequency control

One final tone-based comparison moves only the pitch range downward. The
regular click uses 784 to 523 Hz and the downbeat uses 1047 to 784 Hz. Attack
times, total durations, volume, waveform, scheduling, and accent relationship
remain unchanged.

This intentionally revisits a region that earlier sustained-tone probes found
weak or misleading on the Plus2 buzzer. Testing the short chirps directly will
either identify a usable lower transient or rule out pitch selection as the
path to a less tonal metronome click.

Hardware result: this lower pair improved the timbre and remained useful. A
second boundary probe therefore lowers only the contours again: 523 to 392 Hz
for regular beats and 784 to 523 Hz for the downbeat. This should reveal
whether the improvement continues or crosses into the weak, noisy response
observed in earlier low-frequency experiments.

## Remove the contour

Because lowering the range helped while the earlier high-frequency chirp did
not, the next control removes the two-stage transition. Regular beats use a
stable 523 Hz tone and the downbeat a stable 784 Hz tone, retaining the 6 and
10 ms durations, volume, waveform, and scheduling.

This tests whether the lower pitch alone accounts for the improvement. If it
does, the simpler single-tone click is preferable and the click state can
return to one stop deadline without a mid-click transition.

Hardware result: both single-stage clicks remained audible and synchronized,
and the downbeat remained distinct. They were slightly noisier than the
original high beeps but still pleasant, so the simpler model is viable.

The next focused comparison lowers only the regular click from 523 to 392 Hz.
The downbeat stays at 784 Hz and all articulation, volume, waveform, and timing
parameters remain fixed. This strengthens the accent interval while probing
the lower edge for the non-accented beat alone.

Hardware result: the 392 Hz regular click remained audible. The lower-bound
probe continues at 294 Hz, again changing only the regular frequency while the
784 Hz downbeat and all other parameters remain fixed.

Hardware result: 294 Hz produced a very comfortable and still audible regular
click. With the repeating pulse settled, the next comparison raises only the
downbeat from 784 to 1568 Hz. This reuses the earlier proven G6 accent but keeps
its newer short 10 ms articulation, deliberately favoring an unmistakable bar
marker over matching the softer character of the regular beats.

The next articulation comparison restores a 45 ms duration for both regular
and accented clicks. Frequencies remain 294 and 1568 Hz, respectively, so this
tests how the chosen low/high contrast behaves with equal, substantially fuller
click lengths.
