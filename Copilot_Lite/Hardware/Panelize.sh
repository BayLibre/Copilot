#!/bin/sh

KIKIT=kikit

# Is KiKit installed ?
$KIKIT --version
if [ $? -ne 0 ]
then
	echo "Make sure KiKit is installed. You can install it with the following command : pip install --break-system-packages kikit."
	echo "More information about KiKit are available here : https://github.com/yaqwsx/KiKit."
	exit 1
fi

$KIKIT panelize -p Panel_Config.json Copilot_Lite.kicad_pcb Copilot_Lite_Panel.kicad_pcb

if [ $? -eq 0 ]
then
	echo "Panelization succeeded. The generated file is \"Copilot_Lite_Panel.kicad_pcb\"."
else
	echo "An error occurred."
fi
