#if defined(FK_BOOTLOADER_LARGE)

#include <string.h>

#include "serial5.h"
#include "firmware.h"
#include "phylum.h"
#include "platform.h"

uint8_t firmware_check() {
    FirmwareManager firmware;

    serial5_println("Checking Firmware...");

    if (firmware.open()) {
        if (firmware.check(FirmwareBank::Pending)) {
            firmware.flash(FirmwareBank::Pending);
        }
    }

    platform_board_disable();

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

    platform_board_disable();

    return 0;
}

#endif // defined(FK_BOOTLOADER_LARGE)
