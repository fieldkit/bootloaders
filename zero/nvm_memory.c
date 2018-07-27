#include <string.h>

#include "nvm_memory.h"
#include "serial5.h"

static inline void nvm_ready(void) {
    while (NVMCTRL->INTFLAG.bit.READY == 0) { }
}

void nvm_erase_row(uint32_t *d) {
    nvm_ready();
    NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

    // Execute "ER" Erase Row
    NVMCTRL->ADDR.reg = (uint32_t)d / 2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
    nvm_ready();
}

void nvm_erase_after(uint32_t address) {
    uint32_t PageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
    uint32_t page_size = PageSizes[NVMCTRL->PARAM.bit.PSZ];
    uint32_t pages = NVMCTRL->PARAM.bit.NVMP;
    uint32_t flash_size = page_size * pages;

    while (address < flash_size) {
        // Execute "ER" Erase Row
        NVMCTRL->ADDR.reg = address / 2;
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
        while (NVMCTRL->INTFLAG.bit.READY == 0)
            ;
        address += page_size * 4; // Skip a ROW
    }
}

void nvm_write(uint32_t *d, uint32_t *src, uint32_t nwords) {
    uint32_t minimum = FLASH_PAGE_SIZE >> 2;

    // Set automatic page write
    NVMCTRL->CTRLB.bit.MANW = 0;

    while (nwords > 0) {
        uint32_t len = nwords;
        if (minimum < nwords) {
            len = minimum;
        }
        nwords -= len;

        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        nvm_ready();

        // make sure there are no other memory writes here
        // otherwise we get lock-ups

        while (len--) {
            *d++ = *src++;
        }

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        nvm_ready();
    }
}
