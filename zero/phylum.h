#ifndef _PHYLUM_H_
#define _PHYLUM_H_

#include <phylum/backend.h>
#include <phylum/private.h>
#include <phylum/serial_flash_state_manager.h>
#include <phylum/serial_flash_fs.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include "serial_flash.h"

class TinyFlashStorageBackend : public phylum::StorageBackend {
private:
    flash_memory_t *fmem_;
    phylum::Geometry geometry_;

public:
    TinyFlashStorageBackend(flash_memory_t *fmem) : fmem_(fmem) {
    }

public:
    bool initialize(phylum::sector_index_t sector_size);

public:
    bool open() override;
    bool close() override;
    phylum::Geometry &geometry() override;
    bool erase(phylum::block_index_t block) override;
    bool read(phylum::BlockAddress addr, void *d, size_t n) override;
    bool write(phylum::BlockAddress addr, void *d, size_t n) override;
};

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

struct CoreState : public phylum::MinimumSuperBlock {
    uint32_t time;
    uint32_t seed;
    FirmwareAddresses firmwares;
};

class FirmwareManager {
private:
    flash_memory_t fmem;
    TinyFlashStorageBackend backend{ &fmem };
    phylum::SerialFlashAllocator allocator{ backend };
    phylum::SerialFlashStateManager<CoreState> manager{ backend, allocator };

public:
    bool open();
    bool flash(FirmwareBank bank);
    bool erase(FirmwareBank bank);
};

#endif // _PHYLUM_H_
