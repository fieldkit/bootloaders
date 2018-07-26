#include "board_driver_spi.h"
#include "wiring.h"

#define F_CPU                   48000000L
#define SERCOM_FREQ_REF         48000000
#define SERCOM_NVIC_PRIORITY    ((1 << __NVIC_PRIO_BITS) - 1)
#define SPI_MIN_CLOCK_DIVIDER   (uint8_t)(1 + ((F_CPU - 1) / 12000000))

#define PIN_SPI_MISO         (22u)
#define PIN_SPI_MOSI         (23u)
#define PIN_SPI_SCK          (24u)
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_0
#define PIN_FLASH            (26u)

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
    SERCOM_SPI_MODE_1,		// CPOL : 0  | CPHA : 1
    SERCOM_SPI_MODE_2,		// CPOL : 1  | CPHA : 0
    SERCOM_SPI_MODE_3		// CPOL : 1  | CPHA : 1
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
    while(SERCOM4->SPI.CTRLA.bit.SWRST || SERCOM4->SPI.SYNCBUSY.bit.SWRST);
}

static uint8_t calculate_baudrate(uint32_t baudrate) {
    return SERCOM_FREQ_REF / (2 * baudrate) - 1;
}

static void spi_initialize(SercomSpiTXPad mosi, SercomRXPad miso, SercomSpiCharSize charSize, SercomDataOrder dataOrder, SercomSpiClockMode clockMode, uint32_t baudrate) {
  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

  spi_restart();

  // Setting NVIC
  NVIC_EnableIRQ(SERCOM4_IRQn);
  NVIC_SetPriority(SERCOM4_IRQn, SERCOM_NVIC_PRIORITY);

  // Setting clock
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM4_CORE_Val) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY ) {
  }

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
    while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE) {
    }
}

static void spi_disable() {
    while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE) {
    }
    SERCOM4->SPI.CTRLA.bit.ENABLE = 0;
}

uint8_t spi_transfer(uint8_t data) {
    SERCOM4->SPI.DATA.bit.DATA = data;
    while (SERCOM4->SPI.INTFLAG.bit.RXC == 0) {
    }
    return SERCOM4->SPI.DATA.bit.DATA;
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

void platform_setup() {
    pinMode(5, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(PIN_FLASH, OUTPUT);

    digitalWrite(5, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(PIN_FLASH, HIGH);
}

void spi_open() {
    platform_setup();

    pinPeripheral(PIN_SPI_MISO, g_APinDescription[PIN_SPI_MISO].ulPinType);
    pinPeripheral(PIN_SPI_SCK, g_APinDescription[PIN_SPI_SCK].ulPinType);
    pinPeripheral(PIN_SPI_MOSI, g_APinDescription[PIN_SPI_MOSI].ulPinType);

    spi_config();
}

void spi_close() {
}
