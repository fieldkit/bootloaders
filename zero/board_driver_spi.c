#include "board_driver_spi.h"
#include "wiring.h"
#include "platform.h"

#define SERCOM_NVIC_PRIORITY    ((1 << __NVIC_PRIO_BITS) - 1)
#define SPI_MIN_CLOCK_DIVIDER   (uint8_t)(1 + ((F_CPU - 1) / 12000000))

typedef enum {
    MSB_FIRST = 0,
    LSB_FIRST
} SercomDataOrder;

typedef enum {
    SERCOM_RX_PAD_0 = 0,
    SERCOM_RX_PAD_1,
    SERCOM_RX_PAD_2,
    SERCOM_RX_PAD_3
} SercomRXPad;

typedef enum {
    SERCOM_SPI_MODE_0 = 0,	// CPOL : 0  | CPHA : 0
    SERCOM_SPI_MODE_1,		  // CPOL : 0  | CPHA : 1
    SERCOM_SPI_MODE_2,		  // CPOL : 1  | CPHA : 0
    SERCOM_SPI_MODE_3		    // CPOL : 1  | CPHA : 1
} SercomSpiClockMode;

typedef enum {
    SPI_PAD_0_SCK_1 = 0,
    SPI_PAD_2_SCK_3,
    SPI_PAD_3_SCK_1,
    SPI_PAD_0_SCK_3
} SercomSpiTXPad;

typedef enum {
    SPI_CHAR_SIZE_8_BITS = 0x0ul,
    SPI_CHAR_SIZE_9_BITS
} SercomSpiCharSize;

static void spi_restart() {
    SERCOM4->SPI.CTRLA.bit.SWRST = 1;
    while (SERCOM4->SPI.CTRLA.bit.SWRST || SERCOM4->SPI.SYNCBUSY.bit.SWRST) { }
}

static uint8_t calculate_baudrate(uint32_t baudrate) {
    return F_CPU / (2 * baudrate) - 1;
}

static void spi_initialize(SercomSpiTXPad mosi, SercomRXPad miso, SercomSpiCharSize charSize, SercomDataOrder dataOrder, SercomSpiClockMode clockMode, uint32_t baudrate) {
  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

  spi_restart();

  // Setting NVIC
  NVIC_EnableIRQ(SERCOM4_IRQn);
  NVIC_SetPriority(SERCOM4_IRQn, SERCOM_NVIC_PRIORITY);

  // Setting clock
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM4_CORE_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;

  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) { }

  // Setting the CTRLA register
  SERCOM4->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
                           SERCOM_SPI_CTRLA_DOPO(mosi) |
                           SERCOM_SPI_CTRLA_DIPO(miso) |
                           dataOrder << SERCOM_SPI_CTRLA_DORD_Pos;

  // Setting the CTRLB register
  SERCOM4->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(charSize) | SERCOM_SPI_CTRLB_RXEN;

  // Extract data from clockMode
  int32_t cpha, cpol;

  if ((clockMode & (0x1ul)) == 0 ) {
      cpha = 0;
  }
  else {
      cpha = 1;
  }

  if ((clockMode & (0x2ul)) == 0) {
      cpol = 0;
  }
  else {
      cpol = 1;
  }

  SERCOM4->SPI.CTRLA.reg |=	(cpha << SERCOM_SPI_CTRLA_CPHA_Pos) | (cpol << SERCOM_SPI_CTRLA_CPOL_Pos);
  SERCOM4->SPI.BAUD.reg = calculate_baudrate(baudrate);
}

static void spi_enable() {
    SERCOM4->SPI.CTRLA.bit.ENABLE = 1;
    while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE) { }
}

static void spi_disable() {
    while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE) { }
    SERCOM4->SPI.CTRLA.bit.ENABLE = 0;
}

uint8_t spi_transfer(uint8_t data) {
    SERCOM4->SPI.DATA.bit.DATA = data;
    while (SERCOM4->SPI.INTFLAG.bit.RXC == 0) { }
    return SERCOM4->SPI.DATA.bit.DATA;
}

uint16_t spi_transfer_word(uint16_t data) {
    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } t;

    t.val = data;
    t.msb = spi_transfer(t.msb);
    t.lsb = spi_transfer(t.lsb);

    return t.val;
}

uint8_t spi_transfer_block(void *ptr, size_t n) {
    uint8_t *bytes = (uint8_t *)ptr;
    for (size_t i = 0; i < n; ++i) {
        *bytes = spi_transfer(*bytes);
        bytes++;
    }
    return 0;
}

void spi_config() {
    uint32_t clock = 50000000;
    SercomDataOrder bitOrder = MSB_FIRST;
    SercomSpiClockMode dataMode = SERCOM_SPI_MODE_0;
    uint32_t clockFreq = (clock >= (F_CPU / SPI_MIN_CLOCK_DIVIDER) ? F_CPU / SPI_MIN_CLOCK_DIVIDER : clock);

    spi_disable();
    spi_restart();
    spi_initialize(PAD_SPI_TX, PAD_SPI_RX, SPI_CHAR_SIZE_8_BITS, bitOrder, dataMode, clockFreq);
    spi_enable();
}

uint8_t spi_begin() {
    spi_config();

    return 0;
}

uint8_t spi_end() {
    return 0;
}

void spi_open() {
    pinPeripheral(PIN_SPI_MISO, g_APinDescription[PIN_SPI_MISO].ulPinType);
    pinPeripheral(PIN_SPI_SCK, g_APinDescription[PIN_SPI_SCK].ulPinType);
    pinPeripheral(PIN_SPI_MOSI, g_APinDescription[PIN_SPI_MOSI].ulPinType);

    spi_config();
}

void spi_close() {
}
