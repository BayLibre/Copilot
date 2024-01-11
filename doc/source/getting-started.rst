Getting started
===============

Hardware setup
--------------

To use Copilot, you need:

- A Copilot
- 1 USB-C cable for control (only USB2 wiring needed)

Optionally, depending on your needs:

- 2 USB-C cables for power (power supply <-> Copilot <-> DUT)
- 2 Barrel jack cables for additional power supply
- 3+ wires for FTDI (debug uart)
- Wires for additional GPIOs

Plug it in as following:

.. image:: pictures/copilot_lite_cables.jpg

.. note::
   By default, Copilot has everything turned off. To enable USB power supply, follow steps in :ref:`Example commands`.

.. warning::
   Even if USB-C is a **reversible** connector, the USB-C cables can only work in **one direction**.
   If USB power supply seems not working after enabling. Try flipping one USB-C cable on either ``J9`` or ``J3``.

For additional board documentation, read on the PCB itself.
For design files, see: https://gitlab.baylibre.com/baylibre/copilot/copilot

The latest schematics are available on the `artifacts <https://gitlab.baylibre.com/baylibre/copilot/copilot/-/jobs/artifacts/master/browse?job=kicad>`_ page.

Udev rules
----------

Generally, a linux PC has multiple gpiochips on the system.

To make sure we can easily identify the Copilot, we can match the chip with udev to add some user-friendly symlinks.

#. Add the ``/etc/udev/rules.d/70.copilot.rules`` udev rules to your system:

   .. code-block::

     # Match Copilot USB devices
     SUBSYSTEMS=="usb", ATTRS{manufacturer}=="Bay[lL]ibre", ATTRS{product}=="Bay[lL]ibre Copilot*", ENV{IS_COPILOT}="1", ENV{ID_USB_SERIAL_SHORT}="$attr{serial}"
     # Enable Copilot gpio access via uaccess and copilot symlink
     ENV{IS_COPILOT}=="1", SUBSYSTEMS=="gpio", MODE="0660", SYMLINK+="copilot/by-id/$env{ID_USB_SERIAL_SHORT}/gpiochip", TAG+="uaccess"
     # Create Copilot tty symlink
     ENV{IS_COPILOT}=="1", SUBSYSTEMS=="tty", SYMLINK+="copilot/by-id/$env{ID_USB_SERIAL_SHORT}/tty"

   .. note::

     For users who rely on the ``plugdev`` group, use the following rules instead:

     .. code-block::

         # Match Copilot USB devices
         SUBSYSTEMS=="usb", ATTRS{manufacturer}=="Bay[lL]ibre", ATTRS{product}=="Bay[lL]ibre Copilot*", ENV{IS_COPILOT}="1", ENV{ID_USB_SERIAL_SHORT}="$attr{serial}"
         # Enable Copilot gpio access for the plugdev group and copilot symlink
         ENV{IS_COPILOT}=="1", SUBSYSTEMS=="gpio", MODE="0660", GROUP="plugdev", SYMLINK+="copilot/by-id/$env{ID_USB_SERIAL_SHORT}/gpiochip"
         # Create Copilot tty symlink
         ENV{IS_COPILOT}=="1", SUBSYSTEMS=="tty", SYMLINK+="copilot/by-id/$env{ID_USB_SERIAL_SHORT}/tty"

     ``uaccess`` should be used instead of ``plugdev``.
     For more information, see: https://github.com/systemd/systemd/issues/4288

#. Reload the udev rules with:

  .. prompt:: bash $

     sudo udevadm control --reload
     sudo udevadm trigger

This creates an entry in ``/dev/copilot/by-id/<copilot_id>`` where ``<copilot_id>`` is an unique serial number.

.. _Example commands:

Example commands
----------------

Now that the udev rules have been installed, we can interact with Copilot using the
standard ``libgpiod-utils``, such as ``gpioset``.

Start by find your ``ID_USB_SERIAL_SHORT`` by inspecting ``/dev/copilot/by-id``.
After that export it as an environment variable:

.. prompt:: bash $ auto

   # here, D30HF04Y is an example. Modify for your own serial number accordingly.
   $ export ID_USB_SERIAL_SHORT="D30HF04Y"

By default, USB passthrough is disabled. We can enable USB and the jack barrel power with:

.. prompt:: bash $

   gpioset --hold-period=20ms -t0 --chip /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=1

.. note::

   For older versions of ``libgpiod-utils``, (before v2.0), use the following instead:

   .. prompt:: bash $

      gpioset /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=1


To disable USB and jack power, use ``0=0`` instead:

.. prompt:: bash $

   gpioset --hold-period=20ms -t0 --chip /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=0

Helper script
-------------

``copilot.sh`` is an example of helper script which would allow to either reboot or power off a board connected to Copilot.

It can be used with:

.. prompt:: bash $ auto

   # to reboot
   copilot.sh reboot
   # to poweroff
   copilot.sh poweroff

The contents of ``copilot.sh`` could be as following:

.. code-block:: bash

   #!/bin/bash

   # change accordingly to your own
   ID_USB_SERIAL_SHORT="D30HF04Y"

   usage()
   {
       echo "Usage: ${0##*/} reboot|poweroff"
   }

   if [[ $# -ne 1 ]]; then
       usage
       exit 1
   fi

   do_poweroff()
   {
       gpioset --hold-period=20ms -t0 --chip /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=0
       # or for libgpiod v < 2.0
       # gpioset /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=0
   }

   do_poweron()
   {
       gpioset --hold-period=20ms -t0 --chip /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=1
       # or for libgpiod v < 2.0
       # gpioset /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=1
   }

   do_reboot()
   {
       do_poweroff
       do_poweron
   }

   case "$1" in
       "reboot") do_reboot;;
       "poweroff") do_poweroff;;
       *) usage; exit 1;;

   esac

