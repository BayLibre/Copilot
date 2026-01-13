.. Copilot documentation master file, created by
   sphinx-quickstart on Tue Dec 12 19:07:24 2023.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Copilot's documentation!
===================================

Copilot is a small device used to control the power and UART of a Device Under
Test (DUT). Copilot is controlled and powered by a PC using a single USB-C
connection. It controls the DUT power supply through either another USB-C
connection or a barrel jack connection.

.. image:: pictures/copilot_lite_cables.jpg

Features
--------

 - Power switching with a simple command available in many distributions:

   .. prompt:: bash

     gpioset /dev/copilot/by-id/${ID_USB_SERIAL_SHORT}/gpiochip 0=1

 - Switch power up to 48V/5A via USB-C or Power Jack
 - Connect to DUT UART on 1.8/3.3/5V
 - Provide 3 additional GPIOs for other DUT related tasks

.. toctree::
   :maxdepth: 2
   :caption: Documentation:

   getting-started
   tips-and-tricks
   technical-information

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
