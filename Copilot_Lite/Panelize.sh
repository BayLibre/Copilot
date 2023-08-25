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

$KIKIT panelize \
	--layout 'grid; rows: 2; cols: 1; space: 4mm' \
	--framing 'frame; width: 10mm; space: 2mm; cuts: none' \
	--cuts 'mousebites; drill: 0.5mm; spacing: 0.7mm; offset: -0.25mm' \
	--tabs 'fixed; vcount: 2' \
	--tooling '4hole; hoffset: 10mm; voffset: 5mm; size: 2mm' \
	--fiducials '3fid; hoffset: 8mm; voffset:14mm; coppersize: 2mm; opening: 1mm' \
	--text 'type: simple; text: TOP SIDE; anchor: mt; voffset: 5mm' \
	--post 'copperfill: true; origin: bl; millradius: 1mm' \
	Copilot_Lite.kicad_pcb Copilot_Lite_Panel.kicad_pcb

if [ $? -eq 0 ]
then
	echo "Panelization succeeded. The generated file is \"Copilot_Lite_Panel.kicad_pcb\"."
else
	echo "An error occurred."
fi
