#include <string.h>

#include <phylum/backend.h>
#include <phylum/private.h>
#include <phylum/serial_flash_state_manager.h>
#include <phylum/serial_flash_fs.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include "firmware_header.h"
#include "serial5.h"
#include "phylum.h"

using namespace phylum;

static inline uint32_t get_sf_address(const Geometry &g, BlockAddress a) {
    return (a.block * g.pages_per_block * g.sectors_per_page * g.sector_size) + a.position;
}

class TinyFlashStorageBackend : public StorageBackend {
private:
    flash_memory_t *fmem_;
    Geometry geometry_;

public:
    TinyFlashStorageBackend(flash_memory_t *fmem) : fmem_(fmem) {
    }

public:
    bool initialize(sector_index_t sector_size);

public:
    bool open() override;
    bool close() override;
    Geometry &geometry() override;
    bool erase(block_index_t block) override;
    bool read(BlockAddress addr, void *d, size_t n) override;
    bool write(BlockAddress addr, void *d, size_t n) override;
};

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
    return true;
}

bool TinyFlashStorageBackend::read(BlockAddress addr, void *d, size_t n) {
    auto address = get_sf_address(geometry_, addr);
    flash_read(fmem_, address, d, n);
    return true;
}

bool TinyFlashStorageBackend::write(BlockAddress addr, void *d, size_t n) {
    auto address = get_sf_address(geometry_, addr);
    return true;
}

enum class FirmwareBank {
    CoreA,
    CoreB,
    ModuleA,
    ModuleB,
    NumberOfBanks
};

struct FirmwareAddresses {
    phylum::BlockAddress banks[(size_t)FirmwareBank::NumberOfBanks];
};

struct CoreState : public MinimumSuperBlock {
    uint32_t time;
    uint32_t seed;
    FirmwareAddresses firmwares;
};

uint8_t phylum_open(flash_memory_t *fmem) {
    TinyFlashStorageBackend backend{ fmem };
    SerialFlashAllocator allocator{ backend };
    SerialFlashStateManager<CoreState> manager{ backend, allocator };

    if (!backend.initialize(512)) {
        serial5_printf("Not available");
        return PHYLUM_FAILURE;
    }

    if (!manager.locate()) {
        serial5_printf("Not found");
        return PHYLUM_FAILURE;
    }

    auto addr = manager.location();
    auto &state = manager.state();

    serial5_println("Found SuperBlock! (%lu:%lu)", addr.block, addr.sector);

    auto bank0 = state.firmwares.banks[(int32_t)FirmwareBank::CoreA];
    if (!bank0.valid()) {
        serial5_println("Bank0 is invalid");
        return PHYLUM_SUCCESS;
    }

    AllocatedBlockedFile file{ &backend, OpenMode::Read, &allocator, bank0 };
    file.seek(0);

    firmware_header_t header;
    auto bytes = file.read((uint8_t *)&header, sizeof(firmware_header_t));

    if (header.version != FIRMWARE_VERSION_INVALID) {
        serial5_println("Bank2: version=%d size=%d (%s)", header.version, header.size, header.etag);
    }
    else {
        serial5_println("Bank2: header is invalid!");
    }

    return PHYLUM_SUCCESS;
}

uint8_t phylum_close() {
    return PHYLUM_SUCCESS;
}

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));

void __cxa_pure_virtual(void) {
    while (1) {
    }
}

void __cxa_deleted_virtual(void) {
    while (1) {
    }
}
