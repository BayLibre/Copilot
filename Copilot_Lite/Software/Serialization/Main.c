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
// Private types
//-------------------------------------------------------------------------------------------------
/** An FT-X serie MTP EEPROM header. See FTDI AN_201 document for more details. */
typedef struct __attribute__((packed))
{
	uint16_t Misc_Config;
	uint16_t USB_VID;
	uint16_t USB_PID;
	uint16_t USB_BCD_Release_Number;
	uint8_t Config_Description_Value;
	uint8_t Max_Power;
	uint16_t Device_And_Peripheral_Control;
	uint8_t DBUS_And_CBUS_Control;
	uint8_t Unused_0;
	uint8_t Manufacturer_String_Description_Pointer;
	uint8_t Manufacturer_String_Description_Length;
	uint8_t Product_String_Description_Pointer;
	uint8_t Product_String_Description_Length;
	uint8_t Serial_String_Description_Pointer;
	uint8_t Serial_String_Description_Length;
	uint16_t I2C_Slave_Address;
	uint8_t I2C_Slave_Device_ID[3];
	uint8_t Unused_1;
	uint8_t CBUS_Mux_Control[7];
	uint8_t Unused_2[3];
} TEEPROMHeader;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Detect all connected compatible FTDI devices and make sure only one is connected.
 * @param Pointer_FTDI_Context The initialized FTDI context.
 * @return -1 if an error occurred,
 * @return 0 on success.
 */
static int MainFindDevice(struct ftdi_context *Pointer_FTDI_Context)
{
	struct ftdi_device_list *Pointer_Devices_List, *Pointer_Devices_List_Item;
	int Return_Value = -1, Devices_Count = 0;
	char String_Manufacturer[64], String_Description[64];

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
		if (ftdi_usb_get_strings(Pointer_FTDI_Context, Pointer_Devices_List_Item->dev, String_Manufacturer, sizeof(String_Manufacturer), String_Description, sizeof(String_Description), NULL, 0) < 0)
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

	// Everything went fine
	Return_Value = 0;

Exit_Free_List:
	ftdi_list_free(&Pointer_Devices_List);

	return Return_Value;
}

/** Convert a 8-bit ASCII string to the FTDI string descriptor format.
 * @param Pointer_String_Source_ASCII The string to convert.
 * @param Pointer_Destination_Buffer On output, will contain the corresponding string descriptor.
 * @param Destination_Buffer_Size The destination string descriptor buffer size in bytes.
 * @warning This function does not really convert to UTF-16, make sure to use only 8-bit ASCII characters in the source string.
 * @return -1 if an error occurred,
 * @return 0 on success.
 */
int MainCreateEEPROMStringDescriptor(char *Pointer_String_Source_ASCII, void *Pointer_Destination_Buffer, size_t Destination_Buffer_Size)
{
	uint8_t *Pointer_Destination_Buffer_Bytes = Pointer_Destination_Buffer;
	int Descriptor_Size;

	// The FTDI string descriptor seems to need two bytes at its beginning to store the string size
	if (Destination_Buffer_Size < 2)
	{
		LOG("Error : the destination buffer size must be greater than 2 bytes.\n");
		return -1;
	}
	Pointer_Destination_Buffer_Bytes++; // Bypass the first byte for now (it will contain the string size in bytes, including these two bytes)
	*Pointer_Destination_Buffer_Bytes = 3; // It seems that the second byte always contain the value 3
	Pointer_Destination_Buffer_Bytes++;
	Destination_Buffer_Size--;
	Descriptor_Size = 2;

	// Copy all string characters, adding a zero to atch
	while ((*Pointer_String_Source_ASCII != 0) && (Destination_Buffer_Size > 0))
	{
		*Pointer_Destination_Buffer_Bytes = *Pointer_String_Source_ASCII;
		Pointer_Destination_Buffer_Bytes++;
		*Pointer_Destination_Buffer_Bytes = 0;
		Pointer_Destination_Buffer_Bytes++;
		Pointer_String_Source_ASCII++;
		Destination_Buffer_Size--;
		Descriptor_Size += 2;
	}

	// Fill the descriptor size
	Pointer_Destination_Buffer_Bytes = Pointer_Destination_Buffer;
	*Pointer_Destination_Buffer_Bytes = (uint8_t) Descriptor_Size;

	return Descriptor_Size;
}

/** Serialize the FTDI EEPROM (configure the USB strings, the CBUS pins function, the maximum drawn power and so on.
 * @param Pointer_FTDI_Context The initialized FTDI context.
 * @return -1 if an error occurred,
 * @return 0 on success.
 */
static int MainSerializeEEPROM(struct ftdi_context *Pointer_FTDI_Context)
{
	int Return_Value = -1, Manufacturer_String_Descriptor_Size, Product_String_Descriptor_Size, Serial_Number_String_Descriptor_Size;
	unsigned char EEPROM_Content[MAIN_FTDI_EEPROM_SIZE], Serial_Number_String_Descriptor_Buffer[64], Buffer[128];
	TEEPROMHeader *Pointer_Header;

	// Retrieve the EEPROM content
	printf("Reading EEPROM content...\n");
	if (ftdi_read_eeprom(Pointer_FTDI_Context) < 0)
	{
		LOG("Error : failed to read EEPROM content (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Close_Device;
	}
	if (ftdi_get_eeprom_buf(Pointer_FTDI_Context, EEPROM_Content, sizeof(EEPROM_Content)) < 0)
	{
		LOG("Error : failed to get the read EEPROM buffer (%s).\n", ftdi_get_error_string(Pointer_FTDI_Context));
		goto Exit_Close_Device;
	}
	Pointer_Header = (TEEPROMHeader *) EEPROM_Content;

	// Keep the serial number string (it uses UTF-16 encoding)
	Serial_Number_String_Descriptor_Size = Pointer_Header->Serial_String_Description_Length + 2; // Take into account the prefix bytes of the descriptor
	if (Serial_Number_String_Descriptor_Size >= (int) sizeof(Serial_Number_String_Descriptor_Buffer))
	{
		LOG("Error : the USB serial number string length is too long (%u bytes).\n", Serial_Number_String_Descriptor_Size);
		goto Exit_Close_Device;
	}
	memcpy(Serial_Number_String_Descriptor_Buffer, &EEPROM_Content[Pointer_Header->Serial_String_Description_Pointer], Serial_Number_String_Descriptor_Size); // No need to terminate the string as it is handled as a buffer here

	// Write the manufacturer string descriptor to the String Descriptor Space
	Manufacturer_String_Descriptor_Size = MainCreateEEPROMStringDescriptor(MAIN_USB_MANUFACTURER_STRING, Buffer, sizeof(Buffer));
	if (Manufacturer_String_Descriptor_Size < 0) goto Exit_Close_Device;
	memcpy(&EEPROM_Content[MAIN_FTDI_EEPROM_STRING_DESCRIPTOR_SPACE_OFFSET], Buffer, Manufacturer_String_Descriptor_Size);

	// Write the product string descriptor just after the manufacturer string descriptor
	Product_String_Descriptor_Size = MainCreateEEPROMStringDescriptor(MAIN_USB_PRODUCT_STRING, Buffer, sizeof(Buffer));
	if (Product_String_Descriptor_Size < 0) goto Exit_Close_Device;
	memcpy(&EEPROM_Content[MAIN_FTDI_EEPROM_STRING_DESCRIPTOR_SPACE_OFFSET + Manufacturer_String_Descriptor_Size], Buffer, Product_String_Descriptor_Size);

	// Write back the serial number string descriptor just after the product string descriptor
	memcpy(&EEPROM_Content[MAIN_FTDI_EEPROM_STRING_DESCRIPTOR_SPACE_OFFSET + Manufacturer_String_Descriptor_Size + Product_String_Descriptor_Size], Serial_Number_String_Descriptor_Buffer, Serial_Number_String_Descriptor_Size);

	// TODO

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

	// Configure the required EEPROM settings
	if (MainSerializeEEPROM(Pointer_FTDI_Context) != 0) goto Exit_Free_Library;

	// Everything went fine
	Return_Value = EXIT_SUCCESS;

Exit_Free_Library:
	ftdi_free(Pointer_FTDI_Context);
	return Return_Value;
}
