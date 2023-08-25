/** @file Main.c
 * Serialize a Copilot Lite EEPROM.
 * @author Adrien RICCIARDI
 */
#include <errno.h>
#include <ftdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private constants and macros
//-------------------------------------------------------------------------------------------------
/** TODO */
#define LOG(Format, ...) printf("[%s:%d] " Format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** TODO */
static int MainFindDevice(struct ftdi_context *Pointer_FTDI_Context)
{
	struct ftdi_device_list *Pointer_Devices_List, *Pointer_Devices_List_Item;
	int Return_Value = -1, Devices_Count = 0;
	char String_Manufacturer[64], String_Description[64];

	// Detect all FTDI devices from the FT-X serie
	if (ftdi_usb_find_all(Pointer_FTDI_Context, &Pointer_Devices_List, 0x403, 0x6015) < 0)
	{
		LOG("Error : could not retrieve the list of connected FTDI devices (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		return -1;
	}

	// Determine how many devices were found
	Pointer_Devices_List_Item = Pointer_Devices_List;
	while (Pointer_Devices_List_Item != NULL)
	{
		// Is this device a FT230X or a Copilot Lite ?
		if (ftdi_usb_get_strings(Pointer_FTDI_Context, Pointer_Devices_List_Item->dev, String_Manufacturer, sizeof(String_Manufacturer), String_Description, sizeof(String_Description), NULL, 0) < 0)
		{
			LOG("Error : failed to get the USB strings for an USB device (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
			goto Exit_Free_List;
		}
		if (((strcmp(String_Manufacturer, "FTDI") == 0) && (strcmp(String_Description, "FT230X Basic UART") == 0))
			|| ((strcmp(String_Manufacturer, "BayLibre") == 0) && (strncmp(String_Description, "BayLibre Copilot Lite", 21) == 0))) Devices_Count++;

		Pointer_Devices_List_Item = Pointer_Devices_List_Item->next;
	}
	printf("Found %d Copilot Lite devices.\n", Devices_Count);

	if (Devices_Count == 0)
	{
		printf("No Copilot Lite device was found, please connect one and restart this program.\n");
		goto Exit_Free_List;
	}
	if (Devices_Count > 1)
	{
		printf("More than one Copilot Lite device was found, please connect only one device and restart this program.\n");
		goto Exit_Free_List;
	}

	// Everything went fine
	Return_Value = 0;

Exit_Free_List:
	ftdi_list_free(&Pointer_Devices_List);

	return Return_Value;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	struct ftdi_context *Pointer_FTDI_Context;
	int Return_Value = EXIT_FAILURE;

	// This program must be executed as root, otherwise some udev rules would be required
	if (getuid() != 0)
	{
		LOG("Error : this program must be run as root.\n");
		return EXIT_FAILURE;
	}

	// Initialize the FTDI library
	Pointer_FTDI_Context = ftdi_new();
	if (Pointer_FTDI_Context == NULL)
	{
		LOG("Error : failed to initialize the FTDI library (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		return EXIT_FAILURE;
	}

	// Check whether a single corresponding FTDI device is connected
	printf("Searching for the FTDI device to serialize...\n");
	if (MainFindDevice(Pointer_FTDI_Context) != 0) goto Exit_Free_Library;

	// Everything went fine
	Return_Value = EXIT_SUCCESS;

Exit_Free_Library:
	ftdi_free(Pointer_FTDI_Context);
	return Return_Value;
}
