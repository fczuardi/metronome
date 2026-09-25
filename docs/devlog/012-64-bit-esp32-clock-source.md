# 012 — Use a 64-bit ESP32 clock source

The metronome now samples ESP32's monotonic `esp_timer_get_time()` instead of
casting the 32-bit `micros()` result to `uint64_t`. Casting could not prevent
the source from wrapping after about 71 minutes, which could make the shared
deadline clock observe time moving backwards. The change is applied both when
starting the beat clock and when polling it.

The metronome's real-time late-poll policy remains unchanged: stale clicks are
discarded and playback resumes at the next deadline.

Verification: build the firmware with `pio run -e m5stick-cplus2` and run it
past the former wrap boundary.
