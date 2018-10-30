#if defined(FK_BOOTLOADER_LARGE)

#include <string.h>

#include "firmware.h"
#include "phylum.h"

uint8_t firmware_check() {
    FirmwareManager firmware;

    if (!firmware.open()) {
        return 0;
    }

    if (firmware.check(FirmwareBank::Pending)) {
        firmware.flash(FirmwareBank::Pending);
    }

    return 0;
}

uint8_t firmware_check_before_launch() {
    firmware_check();

    return 0;
}

uint8_t firmware_backups_erase() {
    FirmwareManager firmware;

    if (!firmware.open()) {
        return 0;
    }

    firmware.clear(FirmwareBank::Pending, false);

    return 0;
}

#endif // defined(FK_BOOTLOADER_LARGE)
