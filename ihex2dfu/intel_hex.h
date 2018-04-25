// intel_hex.h

#ifndef __INTEL_HEX_H
#define __INTEL_HEX_H


extern char* ihex_get_last_error(void);
extern bool ihex_read_file(char *filename, uint8_t *buffer, unsigned int buffer_size, uint32_t *image_size);


#endif