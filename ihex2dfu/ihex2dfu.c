// ihex2dfu.c
// Convert an Intel HEX file to a DFU firmware image
//
// Licence: GPLv3
// Author: Paul Qureshi

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "intel_hex.h"
#include "cmdargs.h"
#include "crc.h"

char *input_filename;
char *output_filename;

typedef struct
{
	uint32_t dwCRC;
	uint8_t bLength;
	uint8_t ucDfuSignature[3];
	uint16_t bcdDFU;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
} DFU_SUFFIX_t;

// reverse src and copy to dest
void rmemcpy(uint8_t *dest, uint8_t *src, int size)
{
	dest += size - 1;
	while (size--)
		*dest-- = *src++;
}

int main(int argc, char *argv[])
{
	if (!cmdargs_parse(argc, argv))
	{
		cmdargs_print_help("ihex2dfu");
		return 1;
	}

	// find required buffer size
	const unsigned int max_buffer_size = 1024*1024*100;	// 100MB
	uint32_t image_size = 0;
	if (!ihex_read_file(input_filename, NULL, max_buffer_size, &image_size))
	{
		printf("%s\n", ihex_get_last_error());
		return 1;
	}
	printf("Image size: %u bytes\n", image_size);

	// load firmware image
	uint8_t *image = malloc(image_size + sizeof(DFU_SUFFIX_t));
	if (image == NULL)
	{
		printf("Unable to allocate memory for firmware image.\n");
		return 1;
	}

	if (!ihex_read_file(input_filename, image, image_size, NULL))
	{
		printf("%s\n", ihex_get_last_error());
		free(image);
		return 1;
	}

	// add DFU suffix
	DFU_SUFFIX_t suffix;
	suffix.bLength = sizeof(suffix);
	suffix.ucDfuSignature[2] = 0x55;
	suffix.ucDfuSignature[1] = 0x46;
	suffix.ucDfuSignature[0] = 0x44;
	suffix.idVendor = 0xFFFF;
	suffix.idProduct = 0xFFFF;
	suffix.bcdDevice = 0xFFFF;
	suffix.bcdDFU = 0x0100;

	rmemcpy(image + image_size, (uint8_t *)&suffix, sizeof(suffix));
	suffix.dwCRC = crc32(image, image_size + sizeof(suffix) - sizeof(uint32_t));
	memcpy(image + image_size + sizeof(suffix) - sizeof(suffix.dwCRC), &suffix.dwCRC, sizeof(suffix.dwCRC));

	// write DFU file
	FILE *fout = fopen(output_filename, "wb");
	if (fout == NULL)
	{
		printf("Unable to open \"%s\".\n", output_filename);
		free(image);
		return 1;
	}
	fwrite(image, image_size + sizeof(DFU_SUFFIX_t), 1, fout);
	fclose(fout);
	printf("DFU image written to \"%s\".\n", output_filename);

	free(image);
    return 0;
}

