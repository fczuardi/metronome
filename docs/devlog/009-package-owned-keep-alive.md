# Package-owned PCM keep-alive

## Goal

Replace the metronome's direct M5Unified channel-1 workaround with the new
`m5-tone-output` idle-policy boundary.

## Change

`MetronomeAudio` now constructs its backend with
`AudioIdlePolicy::KeepAlive`. The package owns the silent buffer, virtual
channel, startup, and cleanup. The metronome only selects the policy, sets
master volume, and requests musical PCM playback.

During development, `platformio.ini` consumes the sibling package directly so
the application can validate the API before another registry snapshot is
published. No metronome source file calls `M5.Speaker` or knows which channel
implements keep-alive.

## Verification

```sh
just build
just upload
just monitor
```

On hardware:

1. startup reports `audio: idle_policy=keep_alive ok=yes`;
2. plain-1600 accent and mid-tick regular defaults remain unchanged;
3. volume affects both sounds and zero is silent;
4. no electrical start transient returns between 500 ms beats;
5. Tempo, Volume, and Sound modes retain their existing behavior;
6. click timing and sound selection logs remain aligned.

This slice validates ownership transfer, not whether keep-alive deserves to
exist; that decision was already made from the preceding metronome evidence.

## Hardware result

The M5StickC Plus2 startup reported:

```text
audio: idle_policy=keep_alive ok=yes
```

The package-owned policy preserved the behavior of the former app-local silent
channel: the electrical transient remained absent, both default sounds followed
master-volume changes, and volume zero was silent. PCM events still report
`playback=started` at zero because they are submitted normally and muted by
M5Unified's master mixer; this is expected.

Across the supplied logs, beats stayed exactly 500 ms apart, every ordinary
beat reported `elapsed=1`, and every PCM request returned `ok=yes`. Tempo,
Volume, and Sound interaction remained functional. The ownership transfer is
hardware-validated.
