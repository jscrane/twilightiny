Twilight-3
==========

Twilight with attiny85 and Interrupted.

Status
------
- TSB doesn't work in situ, because it requires the ability to reset
  the processor; maybe add a header to connect a reset switch?
- The PIR socket is not pin-compatible with "Mini PIRs"; fix the layout
  to allow a Mini PIR to be plugged into it.
- PIR, button and LDR are working.
- Power circuitry is untested.
- Firmware needs debugging: it doesn't seem to be possible to turn the
  LED on.
