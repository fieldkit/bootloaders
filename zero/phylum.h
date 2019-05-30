#ifndef _PHYLUM_H_
#define _PHYLUM_H_

#include <phylum/backend.h>
#include <phylum/private.h>
#include <phylum/basic_super_block_manager.h>
#include <phylum/files.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include "serial_flash.h"
#include "core_state.h"
#include "firmware_header.h"

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
    void geometry(phylum::Geometry g) override;
    bool erase(phylum::block_index_t block) override;
    bool read(phylum::BlockAddress addr, void *d, size_t n) override;
    bool write(phylum::BlockAddress addr, void *d, size_t n) override;
    bool eraseAll() override;
};

class FirmwareManager {
private:
    flash_memory_t fmem;
    TinyFlashStorageBackend backend{ &fmem };
    phylum::SerialFlashAllocator allocator{ backend };
    phylum::BasicSuperBlockManager<CoreState> manager{ backend, allocator };

public:
    bool open();
    bool check(FirmwareBank bank);
    bool flash(FirmwareBank bank);
    bool clear(FirmwareBank bank, bool erase_file);

public:
    bool set_existing(firmware_header_t *header);
    bool get_existing(firmware_header_t *header);

};

#endif // _PHYLUM_H_
