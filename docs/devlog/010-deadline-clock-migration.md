# 010 — Migrate the metronome to `DeadlineClock`

## Goal

Use the shared platform-independent deadline scheduler while preserving the
metronome's existing timing behavior.

## Design

The metronome now supplies monotonic microsecond timestamps to
`musical-clock`. `DeadlineClock::poll()` replaces the local `BeatClock` and
continues advancing from absolute deadlines when the loop is delayed.

The first interval is supplied to `begin()`. Tempo changes use
`reschedule(..., PreservePhase)`, which preserves the fraction of the current
interval that remained when the command arrived. This records the selected
physical scheduling policy explicitly without yet introducing a higher-level
musical transport or tempo policy.

## Verification

Host-side `musical-clock` tests cover deadline boundaries, catch-up, and phase
preservation. The metronome firmware should be built and flashed for the
hardware validation of beat regularity and tempo changes.
