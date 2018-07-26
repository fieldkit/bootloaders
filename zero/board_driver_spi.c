#include "board_driver_spi.h"
#include "serial5.h"

#define ID0_WINBOND	0xEF
#define ID0_SPANSION	0x01
#define ID0_MICRON	0x20
#define ID0_MACRONIX	0xC2
#define ID0_SST		0xBF

#define F_CPU                 48000000L
#define SERCOM_FREQ_REF       48000000
#define SERCOM_NVIC_PRIORITY ((1<<__NVIC_PRIO_BITS) - 1)
#define SPI_MIN_CLOCK_DIVIDER (uint8_t)(1 + ((F_CPU - 1) / 12000000))

#define CS_PORT 0
#define CS_PIN 27
#define PIN_SPI_MISO         (22u)
#define PIN_SPI_MOSI         (23u)
#define PIN_SPI_SCK          (24u)
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_0
#define PIN_FLASH            (26u)

typedef enum _EPioType
{
  PIO_NOT_A_PIN=-1,     /* Not under control of a peripheral. */
  PIO_EXTINT=0,         /* The pin is controlled by the associated signal of peripheral A. */
  PIO_ANALOG,           /* The pin is controlled by the associated signal of peripheral B. */
  PIO_SERCOM,           /* The pin is controlled by the associated signal of peripheral C. */
  PIO_SERCOM_ALT,       /* The pin is controlled by the associated signal of peripheral D. */
  PIO_TIMER,            /* The pin is controlled by the associated signal of peripheral E. */
  PIO_TIMER_ALT,        /* The pin is controlled by the associated signal of peripheral F. */
  PIO_COM,              /* The pin is controlled by the associated signal of peripheral G. */
  PIO_AC_CLK,           /* The pin is controlled by the associated signal of peripheral H. */
  PIO_DIGITAL,          /* The pin is controlled by PORT. */
  PIO_INPUT,            /* The pin is controlled by PORT and is an input. */
  PIO_INPUT_PULLUP,     /* The pin is controlled by PORT and is an input with internal pull-up resistor enabled. */
  PIO_OUTPUT,           /* The pin is controlled by PORT and is an output. */

  PIO_PWM=PIO_TIMER,
  PIO_PWM_ALT=PIO_TIMER_ALT,
} EPioType ;

typedef enum _EPortType
{
  NOT_A_PORT=-1,
  PORTA=0,
  PORTB=1,
  PORTC=2,
} EPortType ;

typedef enum
{
    EXTERNAL_INT_0 = 0,
    EXTERNAL_INT_1,
    EXTERNAL_INT_2,
    EXTERNAL_INT_3,
    EXTERNAL_INT_4,
    EXTERNAL_INT_5,
    EXTERNAL_INT_6,
    EXTERNAL_INT_7,
    EXTERNAL_INT_8,
    EXTERNAL_INT_9,
    EXTERNAL_INT_10,
    EXTERNAL_INT_11,
    EXTERNAL_INT_12,
    EXTERNAL_INT_13,
    EXTERNAL_INT_14,
    EXTERNAL_INT_15,
    EXTERNAL_INT_NMI,
    EXTERNAL_NUM_INTERRUPTS,
    NOT_AN_INTERRUPT = -1,
    EXTERNAL_INT_NONE = NOT_AN_INTERRUPT,
} EExt_Interrupts ;

typedef struct _PinDescription
{
    EPortType       ulPort ;
    uint32_t        ulPin ;
    EPioType        ulPinType ;
    uint32_t        ulPinAttribute ;
    EExt_Interrupts ulExtInt ;
} PinDescription ;

#define PIN_ATTR_NONE          (0UL<<0)
#define PIN_ATTR_COMBO         (1UL<<0)
#define PIN_ATTR_ANALOG        (1UL<<1)
#define PIN_ATTR_DIGITAL       (1UL<<2)
#define PIN_ATTR_PWM           (1UL<<3)
#define PIN_ATTR_TIMER         (1UL<<4)
#define PIN_ATTR_TIMER_ALT     (1UL<<5)
#define PIN_ATTR_EXTINT        (1UL<<6)

PinDescription g_APinDescription[] = {
  { PORTA, 11, PIO_SERCOM, (PIN_ATTR_DIGITAL), EXTERNAL_INT_11 }, // RX: SERCOM0/PAD[3]
  { PORTA, 10, PIO_SERCOM, (PIN_ATTR_DIGITAL), EXTERNAL_INT_10 }, // TX: SERCOM0/PAD[2]

  // 2..12
  // Digital Low
  { PORTA, 14, PIO_DIGITAL, (PIN_ATTR_DIGITAL), EXTERNAL_INT_14 },
  { PORTA,  9, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_9 }, // TCC0/WO[1]
  { PORTA,  8, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_NMI },  // TCC0/WO[0]
  { PORTA, 15, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_15 }, // TC3/WO[1]
  { PORTA, 20, PIO_TIMER_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), EXTERNAL_INT_4 }, // TCC0/WO[6]
  { PORTA, 21, PIO_DIGITAL, (PIN_ATTR_DIGITAL), EXTERNAL_INT_5 },

  // Digital High
  { PORTA,  6, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER|PIN_ATTR_ANALOG), EXTERNAL_INT_6 }, // TCC1/WO[0]
  { PORTA,  7, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER|PIN_ATTR_ANALOG), EXTERNAL_INT_7 }, // TCC1/WO[1]
  { PORTA, 18, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_2 }, // TC3/WO[0]
  { PORTA, 16, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_0 }, // TCC2/WO[0]
  { PORTA, 19, PIO_TIMER_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), EXTERNAL_INT_3 }, // TCC0/WO[3]

  // 13 (LED)
  { PORTA, 17, PIO_PWM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_1 }, // TCC2/WO[1]

  // 14..19 - Analog pins
  // --------------------
  { PORTA,  2, PIO_ANALOG, PIN_ATTR_ANALOG, EXTERNAL_INT_2 }, // ADC/AIN[0]
  { PORTB,  8, PIO_ANALOG, (PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_8 }, // ADC/AIN[2]
  { PORTB,  9, PIO_ANALOG, (PIN_ATTR_PWM|PIN_ATTR_TIMER), EXTERNAL_INT_9 }, // ADC/AIN[3]
  { PORTA,  4, PIO_ANALOG, 0, EXTERNAL_INT_4 }, // ADC/AIN[4]
  { PORTA,  5, PIO_ANALOG, 0, EXTERNAL_INT_5 }, // ADC/AIN[5]
  { PORTB,  2, PIO_ANALOG, 0, EXTERNAL_INT_2 }, // ADC/AIN[10]

  // 20..21 I2C pins (SDA/SCL and also EDBG:SDA/SCL)
  // ----------------------
  { PORTA, 22, PIO_SERCOM, PIN_ATTR_DIGITAL, EXTERNAL_INT_6 }, // SDA: SERCOM3/PAD[0]
  { PORTA, 23, PIO_SERCOM, PIN_ATTR_DIGITAL, EXTERNAL_INT_7 }, // SCL: SERCOM3/PAD[1]

  // 22..24 - SPI pins (ICSP:MISO,SCK,MOSI)
  // ----------------------
  { PORTA, 12, PIO_SERCOM_ALT, PIN_ATTR_DIGITAL, EXTERNAL_INT_12 }, // MISO: SERCOM4/PAD[0]
  { PORTB, 10, PIO_SERCOM_ALT, PIN_ATTR_DIGITAL, EXTERNAL_INT_10 }, // MOSI: SERCOM4/PAD[2]
  { PORTB, 11, PIO_SERCOM_ALT, PIN_ATTR_DIGITAL, EXTERNAL_INT_11 }, // SCK: SERCOM4/PAD[3]

  // 25..26 - RX/TX LEDS (PB03/PA27)
  // --------------------
  { PORTB,  3, PIO_OUTPUT, PIN_ATTR_DIGITAL, EXTERNAL_INT_NONE }, // used as output only
  { PORTA, 27, PIO_OUTPUT, PIN_ATTR_DIGITAL, EXTERNAL_INT_NONE }, // used as output only

  // 27..29 - USB
  // --------------------
  { PORTA, 28, PIO_COM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // USB Host enable
  { PORTA, 24, PIO_COM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // USB/DM
  { PORTA, 25, PIO_COM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // USB/DP

  // 30..41 - EDBG
  // ----------------------
  // 30/31 - EDBG/UART
  { PORTB, 22, PIO_SERCOM_ALT, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // TX: SERCOM5/PAD[2]
  { PORTB, 23, PIO_SERCOM_ALT, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // RX: SERCOM5/PAD[3]

  // 32/33 I2C (SDA/SCL and also EDBG:SDA/SCL)
  { PORTA, 22, PIO_SERCOM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // SDA: SERCOM3/PAD[0]
  { PORTA, 23, PIO_SERCOM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // SCL: SERCOM3/PAD[1]

  // 34..37 - EDBG/SPI
  { PORTA, 19, PIO_SERCOM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // MISO: SERCOM1/PAD[3]
  { PORTA, 16, PIO_SERCOM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // MOSI: SERCOM1/PAD[0]
  { PORTA, 18, PIO_SERCOM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // SS: SERCOM1/PAD[2]
  { PORTA, 17, PIO_SERCOM, PIN_ATTR_NONE, EXTERNAL_INT_NONE }, // SCK: SERCOM1/PAD[1]

  // 38..41 - EDBG/Digital
  { PORTA, 13, PIO_PWM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM), EXTERNAL_INT_13 }, // EIC/EXTINT[13] *TCC2/WO[1] TCC0/WO[7]
  { PORTA, 21, PIO_PWM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM), EXTERNAL_INT_NONE }, // Pin 7
  { PORTA,  6, PIO_PWM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM), EXTERNAL_INT_NONE }, // Pin 8
  { PORTA,  7, PIO_PWM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM), EXTERNAL_INT_NONE }, // Pin 9

  // 42 (AREF)
  { PORTA, 3, PIO_ANALOG, PIN_ATTR_ANALOG, EXTERNAL_INT_NONE }, // DAC/VREFP

  // ----------------------
  // 43 - Alternate use of A0 (DAC output)
  { PORTA,  2, PIO_ANALOG, PIN_ATTR_ANALOG, EXTERNAL_INT_2 }, // DAC/VOUT
};

#define LOW             (0x0)
#define HIGH            (0x1)

#define INPUT           (0x0)
#define OUTPUT          (0x1)
#define INPUT_PULLUP    (0x2)
#define INPUT_PULLDOWN  (0x3)

void pinMode(uint32_t ulPin, uint32_t ulMode) {
    // Handle the case the pin isn't usable as PIO
    if (g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN) {
        return ;
    }

    // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
    switch (ulMode) {
    case INPUT: {
        // Set pin to input mode
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN);
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        break;
    }
    case INPUT_PULLUP: {
        // Set pin to input mode with pull-up resistor enabled
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
        PORT->Group[g_APinDescription[ulPin].ulPort].OUTSET.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        break;
    }
    case INPUT_PULLDOWN: {
        // Set pin to input mode with pull-down resistor enabled
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
        PORT->Group[g_APinDescription[ulPin].ulPort].OUTCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        break;
    }
    case OUTPUT: {
        // Enable input, to support reading back values, with pullups disabled
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN);
        // Set pin to output mode
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRSET.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        break;
    }
    default:
        break;
    }
}

int pinPeripheral(uint32_t ulPin, EPioType ulPeripheral) {
    // Handle the case the pin isn't usable as PIO
    if (g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN) {
        return -1;
    }

    switch (ulPeripheral)
    {
    case PIO_DIGITAL:
    case PIO_INPUT:
    case PIO_INPUT_PULLUP:
    case PIO_OUTPUT: {
        // Configure pin mode, if requested
        if (ulPeripheral == PIO_INPUT) {
            pinMode(ulPin, INPUT);
        }
        else
        {
            if (ulPeripheral == PIO_INPUT_PULLUP) {
                pinMode( ulPin, INPUT_PULLUP);
            }
            else {
                if ( ulPeripheral == PIO_OUTPUT) {
                    pinMode(ulPin, OUTPUT);
                }
                else {
                    // PIO_DIGITAL, do we have to do something as all cases are covered?
                }
            }
        }
        break;
    }

    case PIO_ANALOG:
    case PIO_SERCOM:
    case PIO_SERCOM_ALT:
    case PIO_TIMER:
    case PIO_TIMER_ALT:
    case PIO_EXTINT:
    case PIO_COM:
    case PIO_AC_CLK: {
        if (g_APinDescription[ulPin].ulPin & 1) {
            uint32_t temp;
            // Get whole current setup for both odd and even pins and remove odd one
            temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXE(0xF);
            // Set new muxing
            PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXO(ulPeripheral);
            // Enable port mux
            PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN;
        }
        else {
            uint32_t temp;
            temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXO(0xF);
            PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXE(ulPeripheral);
            PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN; // Enable port mux
        }
        break;
    }

    case PIO_NOT_A_PIN: {
        return -1l;
        break;
    }
    }

    return 0l;
}

void digitalWrite(uint32_t ulPin, uint32_t ulVal) {
    // Handle the case the pin isn't usable as PIO
    if (g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN) {
        return ;
    }

    EPortType port = g_APinDescription[ulPin].ulPort;
    uint32_t pin = g_APinDescription[ulPin].ulPin;
    uint32_t pinMask = (1ul << pin);

    if ((PORT->Group[port].DIRSET.reg & pinMask) == 0) {
        // the pin is not an output, disable pull-up if val is LOW, otherwise enable pull-up
        PORT->Group[port].PINCFG[pin].bit.PULLEN = ((ulVal == LOW) ? 0 : 1);
    }

    switch (ulVal) {
    case LOW:
        PORT->Group[port].OUTCLR.reg = pinMask;
        break;

    default:
        PORT->Group[port].OUTSET.reg = pinMask;
        break;
    }

    return;
}

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
    while(SERCOM4->SPI.SYNCBUSY.bit.ENABLE) {
    }
}

static void spi_disable() {
    while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE) {
    }
    SERCOM4->SPI.CTRLA.bit.ENABLE = 0;
}

uint8_t spi_transfer(uint8_t data) {
    SERCOM4->SPI.DATA.bit.DATA = data;
    while( SERCOM4->SPI.INTFLAG.bit.RXC == 0 ) {
    }
    return SERCOM4->SPI.DATA.bit.DATA;
}

static inline void cs_setup() {
    PORT->Group[CS_PORT].DIRSET.reg = (1 << CS_PIN);
}

static inline void cs_assert() {
    PORT->Group[CS_PORT].OUTCLR.reg = (1 << CS_PIN);
}

static inline void cs_release() {
    PORT->Group[CS_PORT].OUTSET.reg = (1 << CS_PIN);
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
    pinMode(5, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(PIN_FLASH, OUTPUT);

    digitalWrite(5, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(PIN_FLASH, HIGH);

    pinPeripheral(PIN_SPI_MISO, g_APinDescription[PIN_SPI_MISO].ulPinType);
    pinPeripheral(PIN_SPI_SCK, g_APinDescription[PIN_SPI_SCK].ulPinType);
    pinPeripheral(PIN_SPI_MOSI, g_APinDescription[PIN_SPI_MOSI].ulPinType);

    spi_config();

    char buf[16] = { 0 };

    spi_begin();
    digitalWrite(PIN_FLASH, LOW);
    spi_transfer(0x9F);
    buf[0] = spi_transfer(0); // manufacturer ID
    buf[1] = spi_transfer(0); // memory type
    buf[2] = spi_transfer(0); // capacity
    if (buf[0] == ID0_SPANSION) {
        buf[3] = spi_transfer(0); // ID-CFI
        buf[4] = spi_transfer(0); // sector size
    }
    digitalWrite(PIN_FLASH, HIGH);
    spi_end();

    serial5_printf("ID: %02X %02X %02X %02X %02X\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
}

void spi_close() {
}
