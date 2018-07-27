#ifndef _PHYLUM_H_
#define _PHYLUM_H_

#include "flash_memory.h"

typedef struct phylum_geometry_ {
    uint16_t sectors_per_block;
    uint16_t sector_size;
} phylum_geometry_t;

typedef struct flash_memory_phylum_t {
    flash_memory_t *fmem;
    phylum_geometry_t g;
} flash_memory_phylum_t;

uint8_t phylum_open(flash_memory_phylum_t *p, flash_memory_t *fmem);

uint8_t phylum_close(flash_memory_phylum_t *p);

#endif // _PHYLUM_H_
