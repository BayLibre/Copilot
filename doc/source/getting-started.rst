Getting started
===============

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

