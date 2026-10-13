Technical information
=====================

Barrel jack power channel
-------------------------

- The Barrel jack power channel is rated for 48V @ 5A without additional
  cooling (at a normal room temperature). A higher power **has not been
  tested** and might blow up your Copilot board, it also may need additional
  cooling.
- The barrel jacks dimensions are 2.1x5.5mm.
- Connectors polarity: the barrel jack outer contact is GND, the center pin is
  a positive voltage.
- Voltage polarity must not be reversed, and a negative voltage on the barrel
  jack center pin is forbidden.
- The minimum allowed voltage on this channel is +5V.
- If a voltage greater than or equal to 53.3V is applied to the jack channel,
  the power transistor will open thanks to an overvoltage protection. The
  Copilot board needs to be fully reset to be used again (by removing all jack
  and USB-C cables connected to the Copilot board and then reconnecting them).
