#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include <stdlib.h>
#include <sam.h>

#include "firmware_header.h"

uint8_t firmware_check_before_launch();

uint8_t firmware_backups_erase();

#endif // _FIRMWARE_H_
