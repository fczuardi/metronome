# 011 — Discard stale clicks after a late poll

The metronome now follows the same real-time policy as the drum sequencer.
`DeadlineClock` reports physical elapsed intervals, but clicks whose deadlines
already passed are not replayed. The beat position advances immediately and
audio resumes on the next absolute deadline, avoiding a compressed recovery
interval or a burst of stale clicks.

The shared result field is named `elapsed_intervals` to keep the physical
clock contract distinct from musical/audio events.
