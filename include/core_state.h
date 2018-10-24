#ifndef _CORE_STATE_H_
#define _CORE_STATE_H_

#include <phylum/private.h>
#include <phylum/super_block_manager.h>

enum class FirmwareBank {
    /**
     * Backup of the original firmware.
     */
    Backup,

    /**
     * Pending firmware, this is where we flash from.
     */
    Pending,

    /**
     * Bank where downloaded module firmware goes.
     */
    Incoming,

    /**
     * Copy of safe firmware.
     */
    Safe,

    /**
     * Reserved for future use.
     */
    Reserved0,

    /**
     * Reserved for future use.
     */
    Reserved1,
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
