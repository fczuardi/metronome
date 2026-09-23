# Metronome

Experimental home for musical-time and metronome interactions on embedded
hardware. The first target is the M5StickC Plus2 with its two user buttons,
135-by-240 display, and onboard passive buzzer.

This repository begins as an application experiment, not a reusable clock or
transport package. Its timing implementation will be compared with the local
`StepClock` in
[`calculator-face-input`](https://github.com/fczuardi/calculator-face-input)
before either design is promoted into shared code.

## Current experiment

The hardware baseline proves the PlatformIO target, M5Unified initialization,
landscape display, and Button A/B edges. A silent-clock slice then added a 120
BPM deadline clock and four visible beat indicators. The current slice adds
generated rim-blend PCM clicks. The two-button control
surface starts in Tempo mode: A increases and B decreases the BPM. Pressing
A+B cycles to Volume mode, where the same buttons adjust output level; the next
chord returns to Tempo. Selectable meter and transport behavior remain
deferred.

```sh
just build
just upload
just monitor
```

`just upload-monitor` flashes and then opens the serial monitor. Run
`just devices` when the serial port is uncertain, and `just clean` to discard
the local PlatformIO build output.

## Timing questions

The metronome is a small second consumer with which to investigate boundaries
also relevant to step sequencers and MIDI playback:

- tempo-independent musical position versus monotonic elapsed time;
- tempo as a conversion between quarter notes and real deadlines;
- meter and downbeat projection versus the underlying pulse;
- Start, Continue, Stop, and reset behavior;
- phase behavior when tempo changes during an interval;
- fractional intervals and long-term drift;
- catch-up after a delayed event loop;
- advancing position while suppressing stale audible clicks;
- independent audio and display consumers of the same timing event.

Standard MIDI Files and MIDI realtime messages are references rather than an
implementation template. SMF separates tick resolution, delta-timed events,
tempo, and time signature. MIDI Timing Clock uses 24 pulses per quarter note
and separate Start, Continue, and Stop messages. A file tick, a realtime MIDI
clock pulse, a sequencer step, and an audible metronome click are related but
not interchangeable concepts.

Useful references:

- [Standard MIDI Files](https://midi.org/standard-midi-files)
- [MIDI realtime messages](https://midi.org/about-midi-part-3midi-messages)

## Candidate boundary

The working vocabulary for the experiments is:

```text
musical position -- tempo --> real-time deadlines
        |
        +-- meter ----------> bar / beat / downbeat
        |
        +-- application ----> click, sequencer step, or timed event

transport ------------------> running state and cursor policy
```

This diagram is not yet an API. In particular, clock progression and output
dispatch may need different catch-up policies: a sequencer must preserve its
position, while a metronome should not emit a burst of clicks whose deadlines
have already passed.

## Planned slices

1. Validate the display and both buttons on hardware. **Complete.**
2. Add a drift-resistant silent beat clock and visible beat position. **Complete.**
3. Add regular and accented downbeat buzzer clicks. **Current.**
4. Add tempo controls and define mid-interval tempo-change behavior.
5. Add selectable meter and click grouping.
6. Add Start, Stop, and Continue interaction.
7. Compare the result with the Calculator sequencer before extracting shared
   timing code.
