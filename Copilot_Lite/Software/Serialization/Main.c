/** @file Main.c
 * Serialize a Copilot Lite EEPROM.
 * @author Adrien RICCIARDI
 */
#include <errno.h>
#include <ftdi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private constants and macros
//-------------------------------------------------------------------------------------------------
/** Display a message prefixed by the function name and the line.
 * @param Format A printf like format string.
 */
#define LOG(Format, ...) printf("[%s:%d] " Format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/** FTDI FT230X USB vendor ID. */
#define MAIN_FTDI_FT230X_USB_VENDOR_ID 0x0403
/** FTDI FT230X USB product ID. */
#define MAIN_FTDI_FT230X_USB_PRODUCT_ID 0x6015
/** FTDI EEPROM size in bytes. */
#define MAIN_FTDI_EEPROM_SIZE 2048
/* The String Descriptor Space area offset in bytes in the FTDI EEPROM. */
#define MAIN_FTDI_EEPROM_STRING_DESCRIPTOR_SPACE_OFFSET 0xA0

/** The USB manufacturer string value to set. */
#define MAIN_USB_MANUFACTURER_STRING "BayLibre"
/** The USB description string value to set. */
#define MAIN_USB_PRODUCT_STRING "BayLibre Copilot Lite V1.1"

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Detect all connected compatible FTDI devices and make sure only one is connected.
 * @param Pointer_FTDI_Context The initialized FTDI context.
 * @param Pointer_String_USB_Serial_Number On output, will contain the USB serial number coming from factory.
 * @param Serial_Number_Buffer_Size The size in bytes of the serial number buffer.
 * @return -1 if an error occurred,
 * @return 0 on success.
 */
static int MainFindDevice(struct ftdi_context *Pointer_FTDI_Context, char *Pointer_String_USB_Serial_Number, size_t Serial_Number_Buffer_Size)
{
	struct ftdi_device_list *Pointer_Devices_List, *Pointer_Devices_List_Item;
	int Return_Value = -1, Devices_Count = 0;
	char String_Manufacturer[64], String_Description[64], String_Serial_Number[64];

	// Detect all FTDI devices from the FT-X serie
	if (ftdi_usb_find_all(Pointer_FTDI_Context, &Pointer_Devices_List, MAIN_FTDI_FT230X_USB_VENDOR_ID, MAIN_FTDI_FT230X_USB_PRODUCT_ID) < 0)
	{
		LOG("Error : could not retrieve the list of connected FTDI devices (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		return -1;
	}

	// Determine how many devices were found
	Pointer_Devices_List_Item = Pointer_Devices_List;
	while (Pointer_Devices_List_Item != NULL)
	{
		// Is this device a FT230X or a Copilot Lite ?
		if (ftdi_usb_get_strings(Pointer_FTDI_Context, Pointer_Devices_List_Item->dev, String_Manufacturer, sizeof(String_Manufacturer), String_Description, sizeof(String_Description), String_Serial_Number, sizeof(String_Serial_Number)) < 0)
		{
			LOG("Error : failed to get the USB strings for an USB device (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
			goto Exit_Free_List;
		}
		if (((strcmp(String_Manufacturer, "FTDI") == 0) && (strcmp(String_Description, "FT230X Basic UART") == 0))
			|| ((strcmp(String_Manufacturer, "BayLibre") == 0) && (strncmp(String_Description, "BayLibre Copilot Lite", 21) == 0))) Devices_Count++;

		Pointer_Devices_List_Item = Pointer_Devices_List_Item->next;
	}
	printf("Found %d Copilot Lite device(s).\n", Devices_Count);

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

	// Open the found device
	if (ftdi_usb_open(Pointer_FTDI_Context, MAIN_FTDI_FT230X_USB_VENDOR_ID, MAIN_FTDI_FT230X_USB_PRODUCT_ID) < 0)
	{
		LOG("Error : could not open the found Copilot Lite device (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Free_List;
	}

	// Store the serial number now that everything is working
	if (strlen(String_Serial_Number) >= Serial_Number_Buffer_Size)
	{
		LOG("Error : not enough room to store the USB serial number.\n");
		goto Exit_Free_List;
	}
	strcpy(Pointer_String_USB_Serial_Number, String_Serial_Number);

	// Everything went fine
	Return_Value = 0;

Exit_Free_List:
	ftdi_list_free(&Pointer_Devices_List);

	return Return_Value;
}

/** Serialize the FTDI EEPROM (configure the USB strings, the CBUS pins function, the maximum drawn power and so on.
 * @param Pointer_FTDI_Context The initialized FTDI context.
 * @param Pointer_String_USB_Serial_Number The USB serial number string to program into the device EEPROM.
 * @return -1 if an error occurred,
 * @return 0 on success.
 */
static int MainSerializeEEPROM(struct ftdi_context *Pointer_FTDI_Context, char *Pointer_String_USB_Serial_Number)
{
	int Return_Value = -1, i;

	// Fill the in-RAM EEPROM structure with the factory settings, so in the same time set the USB strings
	if (ftdi_eeprom_initdefaults(Pointer_FTDI_Context, MAIN_USB_MANUFACTURER_STRING, MAIN_USB_PRODUCT_STRING, Pointer_String_USB_Serial_Number) < 0)
	{
		LOG("Error : could not initialize the FTDI EEPROM (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Close_Device;
	}

	// Set bus power to 500mA
	if (ftdi_set_eeprom_value(Pointer_FTDI_Context, MAX_POWER, 500) < 0)
	{
		LOG("Error : failed to set the maximum power EEPROM setting (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Close_Device;
	}

	// Configure all 4 CBUS pins as GPIOs
	for (i = CBUS_FUNCTION_0; i < CBUS_FUNCTION_4 ; i++)
	{
		if (ftdi_set_eeprom_value(Pointer_FTDI_Context, i, CBUSX_IOMODE) < 0)
		{
			LOG("Error : failed to set the CBUS %d function EEPROM setting (%s).\n", i - CBUS_FUNCTION_0, ftdi_get_error_string(Pointer_FTDI_Context));
			goto Exit_Close_Device;
		}
	}

	// Generate the binary image to write to the EEPROM
	if (ftdi_eeprom_build(Pointer_FTDI_Context) < 0)
	{
		LOG("Error : could not build binary image for the EEPROM (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Close_Device;
	}

	// Update the device EEPROM
	if (ftdi_write_eeprom(Pointer_FTDI_Context) < 0)
	{
		LOG("Error : failed to write EEPROM image to device (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Close_Device;
	}

	// Everything went fine
	Return_Value = 0;

Exit_Close_Device:
	ftdi_usb_close(Pointer_FTDI_Context);

	return Return_Value;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	struct ftdi_context *Pointer_FTDI_Context;
	int Return_Value = EXIT_FAILURE;
	char String_USB_Serial_Number[64];

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
	if (MainFindDevice(Pointer_FTDI_Context, String_USB_Serial_Number, sizeof(String_USB_Serial_Number)) != 0) goto Exit_Free_Library;

	// Configure the required EEPROM settings
	printf("Serializing the device with serial number \"%s\".\n", String_USB_Serial_Number);
	if (MainSerializeEEPROM(Pointer_FTDI_Context, String_USB_Serial_Number) != 0) goto Exit_Free_Library;

	// Everything went fine
	printf("\033[32mSerialization was successful, please reset the FTDI device by disconnecting it.\033[0m\n");
	Return_Value = EXIT_SUCCESS;

Exit_Free_Library:
	ftdi_free(Pointer_FTDI_Context);
	return Return_Value;
}
