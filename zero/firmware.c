#include <string.h>

#include "firmware.h"
#include "serial_flash.h"
#include "nvm_memory.h"
#include "phylum.h"
#include "platform.h"
#include "serial5.h"

uint8_t firmware_flash(flash_memory_t *fmem, firmware_header_t *header) {
    uint32_t PageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
    uint32_t page_size = PageSizes[NVMCTRL->PARAM.bit.PSZ];
    uint32_t pages = NVMCTRL->PARAM.bit.NVMP;
    uint32_t flash_size = page_size * pages;
    uint32_t writing = FIRMWARE_NVM_PROGRAM_ADDRESS;
    uint32_t bytes = 0;

    serial5_println("Flash: Info: page-size=%d pages=%d", page_size, pages);
    serial5_println("Flash: Erasing (0x%x -> 0x%x)", writing, flash_size);

    nvm_erase_after(writing);

    uint32_t reading = header->position;

    while (bytes < header->size) {
        uint8_t buffer[1024];

        serial5_println("Flash: Writing 0x%x -> 0x%x -> 0x%x (%d)", reading, buffer, writing, bytes);
        flash_read(fmem, reading, buffer, sizeof(buffer));

        nvm_write((uint32_t *)writing, (uint32_t *)buffer, sizeof(buffer) / sizeof(uint32_t));

        reading += sizeof(buffer);
        writing += sizeof(buffer);
        bytes += sizeof(buffer);
    }

    serial5_println("Flash: Header 0x%x", FIRMWARE_NVM_HEADER_ADDRESS);

    nvm_write((uint32_t *)FIRMWARE_NVM_HEADER_ADDRESS, (uint32_t *)header, sizeof(firmware_header_t) / sizeof(uint32_t));

    return 0;
}

uint8_t firmware_check() {
    serial5_println("Opening serial flash...");

    flash_memory_t fmem;
    if (!flash_open(&fmem, FLASH_PIN)) {
        serial5_println("Error opening serial flash");
        return 0;
    }

    serial5_println("Opening Phylum...");

    if (!phylum_open(&fmem)) {
        serial5_println("Error opening phylum");
        return 0;
    }

    flash_close(&fmem);

    serial5_flush();

    return 0;
}

uint8_t firmware_check_before_launch() {
    firmware_check();

    return 0;
}

uint8_t firmware_backups_erase() {
    serial5_println("Opening serial flash...");

    flash_memory_t fmem;
    if (!flash_open(&fmem, FLASH_PIN)) {
        serial5_println("Error opening serial flash");
        return 0;
    }

    serial5_println("Erasing banks...");

    flash_close(&fmem);

    return 0;
}
