#ifndef _DRIVER_SERIAL_FLASH_MEMORY_H_
#define _DRIVER_SERIAL_FLASH_MEMORY_H_

#include <sam.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct flash_memory_ {
    uint8_t cs;
    uint32_t capacity;
    uint32_t block_size;
} flash_memory_t;

uint8_t flash_open(flash_memory_t *flash, uint8_t cs);

uint32_t flash_block_size(flash_memory_t *flash);

uint8_t flash_read(flash_memory_t *flash, uint32_t addr, void *buf, uint32_t len);

uint8_t flash_write(flash_memory_t *flash, uint32_t addr, const void *buf, uint32_t len);

uint8_t flash_erase(flash_memory_t *flash, uint32_t addr);

uint8_t flash_close(flash_memory_t *flash);

#ifdef __cplusplus
}
#endif

#endif // _DRIVER_FLASH_MEMORY_H_
