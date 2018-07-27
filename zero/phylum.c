#include <string.h>

#include "phylum.h"
#include "phylum_internal.h"
#include "serial5.h"

#define PHYLUM_CHAIN_LENGTH      (2)
#define PHYLUM_FAILURE           (0)
#define PHYLUM_SUCCESS           (1)

typedef struct flash_location_ {
    block_index_t block;
    uint32_t position;
} flash_location_t;

static uint8_t find_link(flash_memory_phylum_t *p, block_index_t block, SuperBlockLink *found, flash_location_t *where) {
    uint32_t block_size = p->g.sector_size * p->g.sectors_per_block;

    for (uint16_t sector = 0; sector < p->g.sectors_per_block; ++sector) {
        SuperBlockLink link;
        uint32_t address = (block * block_size) + (sector * p->g.sector_size);

        flash_read(p->fmem, address, &link, sizeof(SuperBlockLink));

        if (memcmp(&link.header.magic, MagicKey, sizeof(MagicKey)) == 0) {
            if (found->header.timestamp == TIMESTAMP_INVALID || link.header.timestamp > found->header.timestamp) {
                memcpy(found, &link, sizeof(link));
                where->block = block;
                where->position = (sector * p->g.sector_size);
            }
        }
        else {
            break;
        }
    }

    return PHYLUM_SUCCESS;
}

static uint8_t locate(flash_memory_phylum_t *p, flash_location_t *where) {
    SuperBlockLink link;

    where->block = BLOCK_INDEX_INVALID;
    where->position = 0;

    link.header.timestamp = TIMESTAMP_INVALID;

    for (uint8_t block = 1; block < 3; block++) {
        if (!find_link(p, block, &link, where)) {
            return PHYLUM_FAILURE;
        }
    }

    if (where->block == BLOCK_INDEX_INVALID) {
        return PHYLUM_FAILURE;
    }

    for (uint8_t i = 0; i < PHYLUM_CHAIN_LENGTH + 1; ++i) {
        if (!find_link(p, link.chained_block, &link, where)) {
            return PHYLUM_FAILURE;
        }

        if (where->block == BLOCK_INDEX_INVALID) {
            return PHYLUM_FAILURE;
        }
    }

    return PHYLUM_SUCCESS;
}

uint8_t phylum_open(flash_memory_phylum_t *p, flash_memory_t *fmem) {
    flash_location_t address;

    p->fmem = fmem;
    p->g.sector_size = 512;
    p->g.sectors_per_block = flash_block_size(fmem) / p->g.sector_size;

    if (!locate(p, &address)) {
        serial5_println("phylum: Unable to find SuperBlock");
        return PHYLUM_FAILURE;
    }

    serial5_println("phylum: SuperBlock: (%d.%d)", address.block, address.position / p->g.sector_size);

    return PHYLUM_SUCCESS;
}

uint8_t phylum_close(flash_memory_phylum_t *p) {
    return PHYLUM_SUCCESS;
}
