#ifndef _CORE_STATE_H_
#define _CORE_STATE_H_

#include <phylum/private.h>
#include <phylum/super_block_manager.h>

enum class FirmwareBank {
    Core,
    CoreNew,
    CoreGood,
    Module,
    ModuleNew,
    ModuleGood,
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

#endif
