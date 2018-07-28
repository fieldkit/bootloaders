#include <string.h>

#include "firmware.h"
#include "board_driver_spi.h"
#include "flash_memory.h"
#include "nvm_memory.h"
#include "phylum.h"
#include "platform.h"
#include "serial5.h"

#define FIRMWARE_VERSION_INVALID              ((uint32_t)-1)

#define FLASH_FIRMWARE_BANK_ADDRESS          (1572864)
#define FLASH_FIRMWARE_BANK_SIZE             (256 * 1024)
#define FLASH_FIRMWARE_BANK_1_ADDRESS        (1572864)
#define FLASH_FIRMWARE_BANK_2_ADDRESS        (1572864 + FLASH_FIRMWARE_BANK_SIZE)
#define FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS (FLASH_FIRMWARE_BANK_1_ADDRESS + FLASH_FIRMWARE_BANK_SIZE - sizeof(firmware_header_t))
#define FLASH_FIRMWARE_BANK_2_HEADER_ADDRESS (FLASH_FIRMWARE_BANK_2_ADDRESS + FLASH_FIRMWARE_BANK_SIZE - sizeof(firmware_header_t))

#define NVM_PROGRAM_ADDRESS                  (0x4000)
#define NVM_HEADER_ADDRESS                   ((void *)262144 - 2048)

uint8_t firmware_flash(flash_memory_t *fmem, firmware_header_t *header) {
    uint32_t PageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
    uint32_t page_size = PageSizes[NVMCTRL->PARAM.bit.PSZ];
    uint32_t pages = NVMCTRL->PARAM.bit.NVMP;
    uint32_t flash_size = page_size * pages;
    uint32_t writing = NVM_PROGRAM_ADDRESS;
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

    serial5_println("Flash: Header 0x%x", NVM_HEADER_ADDRESS);

    nvm_write((uint32_t *)NVM_HEADER_ADDRESS, (uint32_t *)header, sizeof(firmware_header_t) / sizeof(uint32_t));

    return 0;
}

uint8_t firmware_check() {
    platform_setup();

    serial5_open();

    serial5_println("");

    serial5_println("Bootloader Ready");

    serial5_println("Opening SPI...");

    spi_open();

    serial5_println("Opening serial flash...");

    flash_memory_t fmem;
    if (!flash_open(&fmem, FLASH_PIN)) {
        serial5_println("Error opening serial flash");
        return 0;
    }

    uint32_t block_size = flash_block_size(&fmem);

    serial5_println("Opening phylum...");

    flash_memory_phylum_t phylum;
    if (!phylum_open(&phylum, &fmem)) {
        serial5_println("Error opening phylum");
        return 0;
    }

    firmware_header_t bank1;
    firmware_header_t bank2;
    flash_read(&fmem, FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS, &bank1, sizeof(bank1));
    flash_read(&fmem, FLASH_FIRMWARE_BANK_2_HEADER_ADDRESS, &bank2, sizeof(bank2));

    bool safe_to_flash = true;

    if (bank1.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Bank1: version=%d size=%d (%s)", bank1.version, bank1.size, bank1.etag);
    }
    else {
        serial5_println("Bank1: header is invalid!");
        safe_to_flash = false;
    }

    if (bank2.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Bank2: version=%d size=%d (%s)", bank2.version, bank2.size, bank2.etag);
    }
    else {
        serial5_println("Bank2: header is invalid!");
    }

    firmware_header_t running;
    memcpy(&running, (void *)NVM_HEADER_ADDRESS, sizeof(running));
    if (running.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Flash: version=%d size=%d (%s)", running.version, running.size, running.etag);
    }
    else {
        serial5_println("Flash: header is invalid!");
        safe_to_flash = false;
    }

    if (safe_to_flash) {
        if (running.version != bank1.version) {
            serial5_println("Flashing! (version=%d)", running.version);
            firmware_flash(&fmem, &bank1);
        }
        else {
            serial5_println("Firmware is good (version=%d) (%s)", running.version, running.etag);
        }
    }

    flash_close(&fmem);

    serial5_flush();

    return 0;
}

uint8_t firmware_check_before_launch() {
    board_initialize();

    firmware_check();

    return 0;
}

