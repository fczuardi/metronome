# Silent visible beat clock

## Goal

Introduce the smallest observable musical clock without mixing timing behavior
with buzzer latency, click duration, tempo controls, or transport commands.

At a fixed 120 BPM, the display should advance through four quarter-note beat
indicators twice per second. Button input must remain responsive without
disturbing the visible cadence.

## Clock design

`BeatClock` owns only a monotonic millisecond deadline. Its next deadline
advances from the preceding deadline rather than from the time at which the
event loop happens to poll it, avoiding cumulative drift from loop and display
latency.

When polling occurs after more than one deadline, `elapsedBeats()` calculates
the complete number of elapsed beats arithmetically and advances the deadline
across all of them. The application advances musical position by that count but
renders only the resulting beat. This is the first separation between clock
progression and output dispatch: stale visual frames are not replayed.

The signed deadline comparison remains safe across `millis()` rollover as long
as a single scheduling interval stays well below half the unsigned 32-bit
range. The initial 500 ms interval easily satisfies that constraint.

This integer-millisecond interval is exact at 120 BPM. It does not yet answer
how arbitrary BPM values should accumulate fractional milliseconds; the tempo
slice will make that limitation observable before choosing a representation.

## Display policy

The screen is fully drawn only during setup. Each beat redraws the old and new
circles, while button edges redraw only the footer status region. Beat one is
yellow to identify the downbeat; beats two through four are cyan.

Serial reports each visible update with its one-based beat index, elapsed beat
count, and current `millis()` value. An elapsed count greater than one exposes a
delayed loop without emitting a burst of stale frames.

## Verification

```sh
just build
just upload
just monitor
```

On the M5StickC Plus2:

1. confirm that the highlighted beat advances `1 2 3 4` at 120 BPM;
2. confirm beat one is yellow and the other beats are cyan;
3. compare Serial timestamps and expect approximately 500 ms between updates;
4. press and release A and B repeatedly while watching for cadence disruption;
5. confirm button feedback changes only the footer and the buzzer remains
   silent;
6. leave the device running long enough to look for visible cumulative drift or
   an unexpected multi-beat catch-up report.

## Hardware result

The first idle run on the M5StickC Plus2 logged beats from `995 ms` through
`39495 ms`. All 78 observations reported `elapsed=1`, the beat index repeated
`1 2 3 4` correctly, and every adjacent timestamp differed by exactly 500 ms.
The final observation remained on the same `495 ms` phase as the first, so this
38.5-second sample showed neither polling jitter nor cumulative deadline drift
at the exact integer interval for 120 BPM.

A button-stress pass then generated hundreds of interleaved A/B press and
release edges, including simultaneous input. Every beat still reported
`elapsed=1`. Most observations remained exactly on the original `495 ms` phase;
isolated observations at `207496`, `213498`, and `221996 ms` arrived 1-3 ms
late, then the following observations returned to the existing deadline phase.
This is bounded event-loop/display jitter rather than cumulative drift.

The device uptime passed 231 seconds without losing beat order or phase, and
both the idle and input-stress targets are accepted on hardware. These results
validate software deadline progression and display cadence at the exact integer
interval for 120 BPM. They do not measure oscillator accuracy against an
external reference or future buzzer onset timing.
