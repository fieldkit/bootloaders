#ifndef _WIRING_H_
#define _WIRING_H_

#include <stdio.h>
#include <stdbool.h>
#include <sam.h>

#define F_CPU           (48000000L)

#define LOW             (0x0)
#define HIGH            (0x1)

#define INPUT           (0x0)
#define OUTPUT          (0x1)
#define INPUT_PULLUP    (0x2)
#define INPUT_PULLDOWN  (0x3)

void pinMode(uint32_t ulPin, uint32_t ulMode);

void digitalWrite(uint32_t ulPin, uint32_t ulVal);

typedef enum _EPioType
{
    PIO_NOT_A_PIN = -1,   /* Not under control of a peripheral. */
    PIO_EXTINT = 0,       /* The pin is controlled by the associated signal of peripheral A. */
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

    PIO_PWM = PIO_TIMER,
    PIO_PWM_ALT = PIO_TIMER_ALT,
} EPioType ;

typedef enum _EPortType
{
    NOT_A_PORT = -1,
    PORTA = 0,
    PORTB = 1,
    PORTC = 2,
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

int pinPeripheral(uint32_t ulPin, EPioType ulPeripheral);

extern PinDescription g_APinDescription[];

#endif
