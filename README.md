View this project on [CADLAB.io](https://cadlab.io/project/1197). 

Twilight-3
==========

Twilight with attiny85 and 
[Interrupted](https://github.com/jscrane/Interrupted).

Status
------
- TSB doesn't work in situ, because it requires the ability to reset
  the processor; maybe add a header to connect a reset switch?
- The PIR socket is not pin-compatible with "Mini PIRs"; fix the layout
  to allow a Mini PIR to be plugged into it.
- Power circuitry is untested.
