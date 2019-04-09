#if defined(FK_BOOTLOADER_LARGE)

#include <string.h>

#include "phylum.h"
#include "firmware_header.h"
#include "serial5.h"
#include "platform.h"
#include "nvm_memory.h"

using namespace phylum;

void operator delete(void * p) {
}

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

void TinyFlashStorageBackend::geometry(phylum::Geometry g) {
    geometry_ = g;
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

    bool success = false;

    for (size_t i = 0; i < sizeof(possible_boards) / sizeof(board_configuration_t); ++i) {
        board_prepare(&possible_boards[i]);

        if (flash_open(&fmem, possible_boards[i].flash_cs)) {
            serial5_println("Flash found on %d (power = %d)", possible_boards[i].flash_cs, possible_boards[i].periph_enable);
            success = true;
            break;
        }
    }

    if (!success) {
        serial5_println("Error opening serial flash");
        return false;
    }

    serial5_println("Opening Phylum...");
    if (!backend.initialize(2048)) {
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

    firmware_header_t header;
    if (!get_existing(&header)) {
        return false;
    }

    if (header.version == FIRMWARE_VERSION_INVALID) {
        serial5_println("No existing firmware state.");
    }
    else {
        serial5_println("Existing: ");
    }

    return true;
}

bool FirmwareManager::check(FirmwareBank bank) {
    auto &state = manager.state();

    auto addr = state.firmwares.banks[(int32_t)bank];
    if (!addr.valid()) {
        serial5_println("Bank %d: address is invalid.", bank);
        return true;
    }

    AllocatedBlockedFile file{ &backend, OpenMode::Read, &allocator, addr };

    file.seek(UINT64_MAX);
    file.seek(0);

    serial5_println("Bank %d: size=%lu", bank, (uint32_t)file.size());

    firmware_header_t header;
    auto bytes_read = file.read((uint8_t *)&header, sizeof(header));
    if (bytes_read != sizeof(header)) {
        return false;
    }

    if (header.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Bank %d: version=%d time=%lu size=%d (%s)", bank, header.version, header.time, header.size, header.etag);
    }
    else {
        serial5_println("Bank %d: header is invalid!", bank);
        return false;
    }

    uint32_t bytes = 0;

    while (bytes < header.size) {
        uint8_t buffer[1024];
        size_t size{ 0 };

        serial5_println("Flash: Reading (%d)", bytes);

        do {
            auto bytes_read = file.read(buffer + size, sizeof(buffer) - size);
            if (bytes_read == 0) {
                break;
            }
            size += bytes_read;
        }
        while (size < sizeof(buffer));

        // NOTE: This is bad, we've reached the end of the file unexpectedly.
        if (size == 0) {
            serial5_println("Unexpected end of file!");
            break;
        }

        bytes += size;
    }

    serial5_println("Flash: Reading (%d)", bytes);

    return true;
}

bool FirmwareManager::flash(FirmwareBank bank) {
    auto &state = manager.state();

    auto addr = state.firmwares.banks[(int32_t)bank];
    if (!addr.valid()) {
        serial5_println("Bank %d: address is invalid.", bank);
        return true;
    }

    AllocatedBlockedFile file{ &backend, OpenMode::Read, &allocator, addr };

    file.seek(UINT64_MAX);
    file.seek(0);

    firmware_header_t header;
    auto bytes_read = file.read((uint8_t *)&header, sizeof(header));
    if (bytes_read != sizeof(header)) {
        return false;
    }

    if (header.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Bank %d: version=%d time=%lu size=%d (%s)", bank, header.version, header.time, header.size, header.etag);
    }
    else {
        serial5_println("Bank %d: header is invalid!", bank);
    }

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

        // NOTE: This is bad, we've reached the end of the file unexpectedly.
        if (size == 0) {
            serial5_println("Unexpected end of file! (%d != %d)", writing, header.size);
            break;
        }

        nvm_write((uint32_t *)writing, (uint32_t *)buffer, size / sizeof(uint32_t));

        writing += size;
        bytes += size;
    }

    serial5_println("Flash: Writing 0x%x (%d)", writing, bytes);

    set_existing(&header);

    return true;
}

bool FirmwareManager::clear(FirmwareBank bank, bool erase_file) {
    auto &state = manager.state();

    serial5_println("Bank %d: Clearing", bank);

    auto addr = state.firmwares.banks[(int32_t)bank];
    if (!addr.valid()) {
        serial5_println("Bank %d: Invalid");
        return true;
    }

    state.firmwares.banks[(int32_t)bank] = { };

    if (!manager.save()) {
        return false;
    }

    if (erase_file) {
        AllocatedBlockedFile file{ &backend, OpenMode::Write, &allocator, addr };

        file.seek(0);

        if (!file.erase_all_blocks()) {
            return false;
        }
    }

    serial5_println("Bank %d: Done", bank);

    return true;
}

bool FirmwareManager::set_existing(firmware_header_t *header) {
    nvm_write((uint32_t *)FIRMWARE_NVM_HEADER_ADDRESS, (uint32_t *)header, sizeof(firmware_header_t) / sizeof(uint32_t));

    return true;
}

bool FirmwareManager::get_existing(firmware_header_t *header) {
    memcpy(header, (uint32_t *)FIRMWARE_NVM_HEADER_ADDRESS, sizeof(firmware_header_t));

    return true;
}

#endif // defined(FK_BOOTLOADER_LARGE)
