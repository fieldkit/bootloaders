#ifndef _DRIVER_FLASH_MEMORY_H_
#define _DRIVER_FLASH_MEMORY_H_

#include <sam.h>

typedef struct flash_memory_ {
    uint8_t cs;
    uint32_t capacity;
    uint32_t block_size;
} flash_memory;

void flash_open(flash_memory *flash, uint8_t cs);

void flash_read(flash_memory *flash, uint32_t addr, void *buf, uint32_t len);

void flash_close(flash_memory *flash);

void nvm_erase_row(uint32_t *d);

void nvm_erase_after(uint32_t address);

void nvm_write(uint32_t *d, uint32_t *src, uint32_t nwords);

#endif // _DRIVER_FLASH_MEMORY_H_
