#include "wiring.h"

#if defined(FK_BOOTLOADER_LARGE)

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

void pinMode(uint32_t ulPin, uint32_t ulMode) {
    // Handle the case the pin isn't usable as PIO
    if (g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN) {
        return ;
    }

    // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
    switch (ulMode) {
    case INPUT: {
        // Set pin to input mode
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg = (uint8_t)(PORT_PINCFG_INEN);
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin);
        break;
    }
    case INPUT_PULLUP: {
        // Set pin to input mode with pull-up resistor enabled
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg = (uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1 << g_APinDescription[ulPin].ulPin);
        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
        PORT->Group[g_APinDescription[ulPin].ulPort].OUTSET.reg = (uint32_t)(1 << g_APinDescription[ulPin].ulPin);
        break;
    }
    case INPUT_PULLDOWN: {
        // Set pin to input mode with pull-down resistor enabled
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg = (uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1 << g_APinDescription[ulPin].ulPin);
        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
        PORT->Group[g_APinDescription[ulPin].ulPort].OUTCLR.reg = (uint32_t)(1 << g_APinDescription[ulPin].ulPin);
        break;
    }
    case OUTPUT: {
        // Enable input, to support reading back values, with pullups disabled
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg = (uint8_t)(PORT_PINCFG_INEN);
        // Set pin to output mode
        PORT->Group[g_APinDescription[ulPin].ulPort].DIRSET.reg = (uint32_t)(1 << g_APinDescription[ulPin].ulPin);
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

#endif
