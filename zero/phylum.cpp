#include <string.h>

#include "phylum.h"
#include "firmware_header.h"
#include "serial5.h"
#include "platform.h"
#include "nvm_memory.h"

using namespace phylum;

static inline uint32_t get_sf_address(const Geometry &g, BlockAddress a) {
    return (a.block * g.pages_per_block * g.sectors_per_page * g.sector_size) + a.position;
}

bool TinyFlashStorageBackend::initialize(sector_index_t sector_size) {
    auto capacity = fmem_->capacity;
    auto block_size = fmem_->block_size;
    if (capacity == 0 || block_size == 0) {
        return false;
    }

    auto sectors_per_page = (page_index_t)4;
    auto pages_per_block = (page_index_t)(block_size / (sectors_per_page * sector_size));
    auto number_of_blocks = (block_index_t)(capacity / block_size);

    if (number_of_blocks > SerialFlashAllocator::MaximumBlocks) {
        number_of_blocks = SerialFlashAllocator::MaximumBlocks;
    }

    geometry_ = Geometry{ number_of_blocks, pages_per_block, sectors_per_page, sector_size };

    return true;
}

bool TinyFlashStorageBackend::open() {
    return true;
}

bool TinyFlashStorageBackend::close() {
    return true;
}

Geometry &TinyFlashStorageBackend::geometry() {
    return geometry_;
}

bool TinyFlashStorageBackend::erase(block_index_t block) {
    auto address = get_sf_address(geometry_, BlockAddress{ block, 0 });
    flash_erase(fmem_, address);
    return true;
}

bool TinyFlashStorageBackend::read(BlockAddress addr, void *d, size_t n) {
    auto address = get_sf_address(geometry_, addr);
    flash_read(fmem_, address, d, n);
    return true;
}

bool TinyFlashStorageBackend::write(BlockAddress addr, void *d, size_t n) {
    auto address = get_sf_address(geometry_, addr);
    flash_write(fmem_, address, d, n);
    return true;
}

bool FirmwareManager::open() {
    serial5_println("Opening serial flash...");
    if (!flash_open(&fmem, FLASH_PIN)) {
        serial5_println("Error opening serial flash");
        return false;
    }

    serial5_println("Opening Phylum...");
    if (!backend.initialize(512)) {
        serial5_println("Error opening Phylum");
        return false;
    }

    if (!manager.locate()) {
        serial5_println("No super block");
        return true;
    }

    auto addr = manager.location();
    auto &state = manager.state();

    serial5_println("Found SuperBlock! (%lu:%lu)", addr.block, addr.sector);

    return true;
}

bool FirmwareManager::flash(FirmwareBank bank) {
    auto &state = manager.state();

    auto addr = state.firmwares.banks[(int32_t)bank];
    if (!addr.valid()) {
        return true;
    }

    AllocatedBlockedFile file{ &backend, OpenMode::Read, &allocator, addr };

    file.seek(0);

    firmware_header_t header;
    auto bytes_read = file.read((uint8_t *)&header, sizeof(header));
    if (bytes_read != sizeof(header)) {
        return false;
    }

    if (header.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Bank %d: version=%d size=%d (%s)", bank, header.version, header.size, header.etag);
    }
    else {
        serial5_println("Bank %d: header is invalid!", bank);
    }

    return true;

    uint32_t PageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
    uint32_t page_size = PageSizes[NVMCTRL->PARAM.bit.PSZ];
    uint32_t pages = NVMCTRL->PARAM.bit.NVMP;
    uint32_t flash_size = page_size * pages;
    uint32_t writing = FIRMWARE_NVM_PROGRAM_ADDRESS;
    uint32_t bytes = 0;

    serial5_println("Flash: Info: page-size=%d pages=%d", page_size, pages);
    serial5_println("Flash: Erasing (0x%x -> 0x%x)", writing, flash_size);

    nvm_erase_after(writing);

    while (bytes < header.size) {
        uint8_t buffer[1024];
        size_t size{ 0 };

        serial5_println("Flash: Writing 0x%x (%d)", writing, bytes);

        do {
            auto bytes_read = file.read(buffer + size, sizeof(buffer) - size);
            if (bytes_read == 0) {
                break;
            }
            size += bytes_read;
        }
        while (size < sizeof(buffer));

        nvm_write((uint32_t *)writing, (uint32_t *)buffer, size / sizeof(uint32_t));

        writing += size;
        bytes += size;
    }

    serial5_println("Flash: Writing 0x%x (%d)", writing, bytes);

    return true;
}

bool FirmwareManager::erase(FirmwareBank bank) {
    auto &state = manager.state();

    serial5_println("Bank %d: Erasing", bank);

    auto addr = state.firmwares.banks[(int32_t)bank];
    if (!addr.valid()) {
        serial5_println("Bank %d: Invalid");
        return true;
    }

    state.firmwares.banks[(int32_t)bank] = { };

    if (!manager.save()) {
        return false;
    }

    AllocatedBlockedFile file{ &backend, OpenMode::Write, &allocator, addr };

    file.seek(0);

    if (!file.erase_all_blocks()) {
        return false;
    }

    serial5_println("Bank %d: Done", bank);

    return true;
}
