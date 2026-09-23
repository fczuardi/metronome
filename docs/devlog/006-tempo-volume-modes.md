# Tempo and volume modes

## Goal

Replace the temporary click-audition controls with a scalable two-button
interaction: A increases, B decreases, and the A+B chord changes what those
buttons control.

## Interaction

The device starts in Tempo mode at 120 BPM:

- release A to add 1 BPM;
- release B to subtract 1 BPM;
- press A+B to enter Volume mode.

Volume starts at 128 and changes in steps of 8. Another A+B chord returns to
Tempo mode. Tempo is bounded to 30–300 BPM and volume to 0–255. The current
mode, value, and button directions remain visible on screen.

Single-button commands execute on release. This gives the second button time
to join the first before either adjustment is committed. Once a chord or
single-button command fires, both buttons must be released before input is
armed again. A recognized chord therefore cannot leak an unwanted value
change.

Hold-to-repeat and acceleration are deliberately deferred until single-step
and chord recognition survive hardware testing.

## Tempo phase policy

Changing BPM does not restart the beat or bar. `BeatClock::setIntervalMs()`
scales the time remaining to the next beat by the ratio between the new and old
intervals. For example, changing tempo halfway through a beat leaves the clock
halfway through that beat under the new duration.

The loop advances any due beat before handling input, preventing an adjustment
at a deadline from silently skipping that beat. This phase-preserving policy
is the first mutable-tempo behavior to compare later with the Calculator
sequencer.

## Verification

```sh
just build
just upload
just monitor
```

On hardware:

1. tap A and B separately and confirm 1 BPM changes in the expected direction;
2. chord A+B repeatedly and confirm Tempo/Volume alternation without stray
   adjustments;
3. in Volume mode, verify steps of 8 and audible level changes;
4. test the 30/300 BPM and 0/255 volume boundaries;
5. change BPM early and late within beats and confirm there is no reset,
   doubled click, or conspicuous pause;
6. mash overlapping A/B presses and confirm each gesture produces at most one
   command;
7. confirm ordinary click records remain `ok=yes` and beat order remains
   correct.

Serial timestamps observe event-loop dispatch, not physical acoustic onset.

## Volume investigation

The first hardware pass found that changing volume clearly affected the
rim-blend accent but appeared not to affect low-knock regular beats; low knock
even remained audible with the displayed value at zero. That contradicts the
backend contract because `M5ToneOutputCore::setVolume()` writes M5Unified's
global master volume, which should apply to every virtual channel.

The diagnostic build reasserted volume immediately before each PCM request and
recorded the app value alongside M5Unified's actual master and channel-0
values. At displayed volume zero, every beat consistently reported app and
master volume zero, channel volume 255, and successful playback. Rim blend was
muted, but low knock remained audible. The state was therefore not being lost;
the remaining sound is consistent with a waveform- or hardware-specific
output transient/floor.

The metronome now applies its control deterministically in the PCM domain.
Before each click, it copies the selected immutable recipe into one fixed
2560-byte scratch buffer with a squared 0–255 gain curve, approximating
M5Unified's master-volume response. The backend stays at full master gain. At
volume zero the app does not request playback at all, guaranteeing that mute
cannot trigger an amplifier or playback-boundary transient. Click records use
`gain=pcm` and distinguish `playback=started` from `playback=muted`.

The 160 ms sample is shorter than the minimum 200 ms beat interval at 300 BPM,
so the scratch buffer is never rewritten while its previous click should still
be playing. This remains an app-level experiment; a gain-bearing sample API
should not be promoted to `m5-tone-output` without another real consumer.

The PCM-gain hardware pass did guarantee silence at volume zero by skipping
playback, but low knock still produced an apparently constant-level click at
nonzero low levels. At those levels the squared 8-bit scaling makes the musical
waveform extremely small or even entirely zero. A click that remains constant
there is likely a physical output transition as M5Unified starts and stops its
speaker task, rather than the low-knock samples themselves.

To test that hypothesis, the next diagnostic loops a 256-sample zero buffer on
M5Unified channel 1 while click one-shots continue through the package-owned
channel 0. This silent keep-alive should keep the output path running between
beats without adding audio. If the constant click disappears and PCM gain
becomes perceptible, the cause is the hardware/backend wake/sleep boundary.
Direct use of channel 1 is intentionally an app-local probe, not a proposed
package contract.

## Keep-alive hardware result

The silent channel-1 stream started successfully and changed the behavior as
predicted. With the output path held active, low knock finally became quieter
as the PCM gain value decreased. This confirms that the previously invariant
click was primarily an electrical/output-start transient rather than the
leading edge encoded in the low-knock PCM data.

This also explains why adding a smooth 12 ms attack to the recipe had produced
barely any audible difference: that experiment changed the PCM envelope but
not the dominant hardware transition. The preferred earlier "low knock" was a
compound sound made from the generated 260 Hz body and the output path waking
for each one-shot.

The clean PCM low knock has its own hardware limit. At low amplitude, some
frequencies fall below the useful response of the M5StickC Plus2 transducer and
become noise-like rather than simply quieter. In this test the recognizable
low-knock character emerged only above roughly volume 160 on the current
squared PCM-gain curve. Volume is therefore not perceptually independent of
timbre on this hardware.

The finding leaves two explicit design choices for a later slice:

- keep the audio path active and select sounds/gain mappings that remain useful
  across the intended volume range; or
- deliberately treat the wake transient as part of a hardware-specific click
  renderer, accepting that ordinary gain control cannot scale it naturally.

The first option offers predictable mute and gain behavior but costs continuous
audio activity and changes the sound that originally won the listening test.
Before either behavior becomes a package feature, the finalist click recipes
should be auditioned again with keep-alive enabled.

## Slice conclusion

The A+B Tempo/Volume switch and its release-to-rearm behavior were validated on
hardware. The slice keeps the silent channel-1 stream and PCM-domain gain,
which together make volume changes deterministic and eliminate the output-start
transient from the musical click.

Both accented and regular beats now use rim blend. This is intentionally a
neutral temporary default: the earlier low-knock preference depended heavily
on the electrical trigger that keep-alive removes. The temporary four-sound
catalog and selection state were removed from the application. A dedicated
click-sound control mode is the next opportunity to audition recipes under the
clean continuous-output conditions and choose honest accent/regular defaults.
