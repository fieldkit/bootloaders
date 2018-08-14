#ifndef _CORE_STATE_H_
#define _CORE_STATE_H_

#include <phylum/private.h>
#include <phylum/wandering_block_manager.h>

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

#endif
