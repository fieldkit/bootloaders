#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include <stdlib.h>
#include <sam.h>

#include "firmware_header.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t firmware_check_before_launch();

uint8_t firmware_backups_erase();

#ifdef __cplusplus
}
#endif

#endif // _FIRMWARE_H_
