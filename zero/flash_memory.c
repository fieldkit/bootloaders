#include <string.h>

#include "flash_memory.h"
#include "board_driver_spi.h"
#include "serial5.h"
#include "wiring.h"

#define ID0_WINBOND	0xEF
#define ID0_SPANSION	0x01
#define ID0_MICRON	0x20
#define ID0_MACRONIX	0xC2
#define ID0_SST		0xBF

static __inline__ void delay_microseconds(unsigned int usec) {
    if (usec == 0) {
        return;
    }

    /*
     *  The following loop:
     *
     *    for (; ul; ul--) {
     *      __asm__ volatile("");
     *    }
     *
     *  produce the following assembly code:
     *
     *    loop:
     *      subs r3, #1        // 1 Core cycle
     *      bne.n loop         // 1 Core cycle + 1 if branch is taken
     */

    // VARIANT_MCK / 1000000 == cycles needed to delay 1uS
    //                     3 == cycles used in a loop
    uint32_t n = usec * (F_CPU / 1000000) / 3;
    __asm__ __volatile__(
        "1:              \n"
        "   sub %0, #1   \n" // substract 1 from %0 (n)
        "   bne 1b       \n" // if result is not 0 jump to 1
        : "+r" (n)           // '%0' is n variable with RW constraints
        :                    // no input
        :                    // no clobber
        );
    // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
    // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Volatile
}

static inline void flash_take(flash_memory *flash) {
    digitalWrite(flash->cs, LOW);
}

static inline void flash_release(flash_memory *flash) {
    digitalWrite(flash->cs, HIGH);
}

static uint32_t flash_read_id(flash_memory *flash, uint8_t *id) {
    spi_begin();
    flash_take(flash);
    spi_transfer(0x9F);
    id[0] = spi_transfer(0); // manufacturer ID
    id[1] = spi_transfer(0); // memory type
    id[2] = spi_transfer(0); // capacity
    if (id[0] == ID0_SPANSION) {
        id[3] = spi_transfer(0); // ID-CFI
        id[4] = spi_transfer(0); // sector size
    }
    flash_release(flash);
    spi_end();

    return 0;
}

static uint32_t flash_calculate_capacity(flash_memory *flash) {
    uint8_t id[16] = { 0 };

    flash_read_id(flash, id);

    uint32_t n = 0;
    if (id[2] >= 16 && id[2] <= 31) {
        n = 1ul << id[2];
    } else {
        if (id[2] >= 32 && id[2] <= 37) {
            n = 1ul << (id[2] - 6);
        } else {
            if ((id[0]==0 && id[1]==0 && id[2]==0) || 
                (id[0]==255 && id[1]==255 && id[2]==255)) {
                n = 0;
            }
        }
    }

    return n;
}

static uint32_t flash_block_size() {
    /*
    if (flags & FLAG_256K_BLOCKS) {
        return 262144;
    }
    */
    return 65536;
}

void flash_wait(flash_memory *flash) {
    uint32_t status;
    while (1) {
        spi_begin();
        flash_take(flash);
        /*
        if (flags & FLAG_STATUS_CMD70) {
            // some Micron chips require this different
            // command to detect program and erase completion
            spi_transfer(0x70);
            status = spi_transfer(0);
            flash_release(flash);
            spi_end();
            if ((status & 0x80)) {
                break;
            }
            } else*/ {
            // All others work by simply reading the status reg
            spi_transfer(0x05);
            status = spi_transfer(0);
            flash_release(flash);
            spi_end();
            if (!(status & 1)) {
                break;
            };
        }
    }
    // busy = 0;
}

void flash_read(flash_memory *flash, uint32_t addr, void *buf, uint32_t len) {
    uint8_t *p = (uint8_t *)buf;
    uint8_t b, f, status, cmd;
    uint8_t flags = 0;
    uint8_t busy = 0;

    memset(p, 0, len);
    f = flags;
    spi_begin();
    // SPIPORT.beginTransaction(SPICONFIG);
    b = busy;
    if (b) {
        // read status register ... chip may no longer be busy
        flash_take(flash);
        if (/*flags & FLAG_STATUS_CMD70*/false) {
            spi_transfer(0x70);
            status = spi_transfer(0);
            if ((status & 0x80)) b = 0;
        } else {
            spi_transfer(0x05);
            status = spi_transfer(0);
            if (!(status & 1)) b = 0;
        }
        flash_release(flash);
        if (b == 0) {
            // chip is no longer busy :-)
            busy = 0;
        } else if (b < 3) {
            // TODO: this may not work on Spansion chips
            // which apparently have 2 different suspend
            // commands, for program vs erase
            flash_take(flash);
            spi_transfer(0x06); // write enable (Micron req'd)
            flash_release(flash);
            delay_microseconds(1);
            cmd = 0x75; //Suspend program/erase for almost all chips
            // but Spansion just has to be different for program suspend!
            if (/*(f & FLAG_DIFF_SUSPEND) &&*/(b == 1)) {
                cmd = 0x85;
            }
            flash_take(flash);
            spi_transfer(cmd); // Suspend command
            flash_release(flash);
            if (/*f & FLAG_STATUS_CMD70*/false) {
                // Micron chips don't actually suspend until flags read
                flash_take(flash);
                spi_transfer(0x70);
                do {
                    status = spi_transfer(0);
                }
                while (!(status & 0x80));
                flash_release(flash);
            } else {
                flash_take(flash);
                spi_transfer(0x05);
                do {
                    status = spi_transfer(0);
                }
                while ((status & 0x01));
                flash_release(flash);
            }
        } else {
            // chip is busy with an operation that can not suspend
            spi_end();	// is this a good idea?
            flash_wait(flash);			// should we wait without ending
            b = 0;			// the transaction??
            spi_begin();
        }
    }

    do {
        uint32_t rdlen = len;
        if (/*f & FLAG_MULTI_DIE*/false) {
            if ((addr & 0xFE000000) != ((addr + len - 1) & 0xFE000000)) {
                rdlen = 0x2000000 - (addr & 0x1FFFFFF);
            }
        }
        flash_take(flash);
        // TODO: FIFO optimize....
        if (/*f & FLAG_32BIT_ADDR*/false) {
            spi_transfer(0x03);
            spi_transfer_word(addr >> 16);
            spi_transfer_word(addr);
        } else {
            spi_transfer_word(0x0300 | ((addr >> 16) & 255));
            spi_transfer_word(addr);
        }
        spi_transfer_block(p, rdlen);
        flash_release(flash);
        p += rdlen;
        addr += rdlen;
        len -= rdlen;
    }
    while (len > 0);

    if (b) {
        flash_take(flash);
        spi_transfer(0x06); // write enable (Micron req'd)
        flash_release(flash);
        delay_microseconds(1);
        cmd = 0x7A;
        if (/*(f & FLAG_DIFF_SUSPEND) &&*/(b == 1)) {
            cmd = 0x8A;
        }
        flash_take(flash);
        spi_transfer(cmd); // Resume program/erase
        flash_release(flash);
    }

    spi_end();
}

void flash_open(flash_memory *flash, uint8_t cs) {
    flash->cs = cs;

    uint32_t size = flash_calculate_capacity(flash);

    serial5_println("Size: %lu", size);
}

void flash_close(flash_memory *flash) {
}

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
