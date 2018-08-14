#ifndef _PHYLUM_H_
#define _PHYLUM_H_

#include "serial_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PHYLUM_FAILURE           (0)
#define PHYLUM_SUCCESS           (1)

uint8_t phylum_open(flash_memory_t *fmem);

uint8_t phylum_close();

#ifdef __cplusplus
}
#endif

#endif // _PHYLUM_H_
