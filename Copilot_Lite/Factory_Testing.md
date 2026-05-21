# Factory testing

The board comes from the factory, fully assembled but without any jumper.  

The test setup is made of :

* A Linux PC running Debian
* An USB 3.x/2.0 hub.
* Two USB-C cables, one of them being *USB 3.x* compliant. The second one can be only *USB 2.0* compliant.

## Serialization
1. On the PC, run `sudo dmesg -wH` in a terminal.
2. Connect the PC to the board `PC USB (J4)` connector through an USB-C cable (the cable can either be *USB 3.x* or *USB 2.0* compliant).
    1. The `PC USB (D2)` LED must light.
    2. The PC must detect a `FT230X Basic UART`.
3. Disconnect the USB-C cable from the board only, flip it face-to-down and reconnect it to the board `PC USB` connector. The result must be identical to the previous step.
4. Run the serialization tool with the command `sudo Copilot/Copilot_Lite/Software/Serialization/copilot-lite-serialize`.
5. Disconnect and reconnect the USB-C cable from the board `PC USB` connector. The `dmesg` output must now display a `BayLibre Copilot Lite Vx.y` product.
6. Make sure that the serialized version is matching the version written on the board PCB silkscreen.

You can close the terminal displaying the kernel console output once all boards have been successfully serialized.

## USB 3.0 passthrough

1. On the PC, run `sudo dmesg -wH | grep -i -e superspeed -e high-speed` in a terminal.
2. Fit a jumper between pins 1 and 3 of the board `J1` pin header.
3. Connect the board `PC USB (J4)` connector to the PC with an USB-C cable.
    1. The `PC USB (D2)` LED must light.
    2. The `USB (D3)` LED must light.
4. Connect an *USB 3.x* USB hub to the `J9` USB-C connector.
5. Connect a second USB-C cable from the PC to the board `J3` USB-C connector. Note that the cable **must** be *USB 3.x* compliant in order to the whether the board *USB 3.0* SuperSpeed pairs are working.
6. There are only two board USB-C connectors orientation that allow the power and the data to go through. Flip the `J3` and the `J9` cable orientation one after the other, the USB hub must be detected in two of the four possible combinations.
7. Each time that the USB hub is detected, make sure that the PC kernel console is detecting both *SuperSpeed* and *high-speed* internal USB hubs. This proves that either the *USB 3.0* and the *USB 2.0* board signals are working.

## Connecting jumpers

1. Remove any testing jumper from the board.
2. `J1` pin header :
    1. Fit a jumper horizontally between pins 3 and 5.
    2. Fit a jumper horizontally between pins 4 and 6.
3. `J11` pin header :
    1. Fit a jumper horizontally between pins 2 and 3.
4. `J12` pin header :
    1. Fit a jumper vertically between pins 3 and 4.