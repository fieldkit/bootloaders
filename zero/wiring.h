#ifndef _WIRING_H_
#define _WIRING_H_

#include <stdio.h>
#include <stdbool.h>
#include <sam.h>

#define LOW             (0x0)
#define HIGH            (0x1)

#define INPUT           (0x0)
#define OUTPUT          (0x1)
#define INPUT_PULLUP    (0x2)
#define INPUT_PULLDOWN  (0x3)

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

/* Definitions and types for pins */
typedef enum _EAnalogChannel
{
    No_ADC_Channel=-1,
    ADC_Channel0=0,
    ADC_Channel1=1,
    ADC_Channel2=2,
    ADC_Channel3=3,
    ADC_Channel4=4,
    ADC_Channel5=5,
    ADC_Channel6=6,
    ADC_Channel7=7,
#if defined __SAMD21J18A__
    ADC_Channel8=8,
    ADC_Channel9=9,
#endif // __SAMD21J18A__
    ADC_Channel10=10,
    ADC_Channel11=11,
#if defined __SAMD21J18A__
    ADC_Channel12=12,
    ADC_Channel13=13,
    ADC_Channel14=14,
    ADC_Channel15=15,
#endif // __SAMD21J18A__
    ADC_Channel16=16,
    ADC_Channel17=17,
    ADC_Channel18=18,
    ADC_Channel19=19,
    DAC_Channel0,
} EAnalogChannel ;

// Definitions for TC channels
typedef enum _ETCChannel
{
    NOT_ON_TIMER=-1,
    TCC0_CH0 = (0<<8)|(0),
    TCC0_CH1 = (0<<8)|(1),
    TCC0_CH2 = (0<<8)|(2),
    TCC0_CH3 = (0<<8)|(3),
    TCC0_CH4 = (0<<8)|(0), // Channel 4 is 0!
    TCC0_CH5 = (0<<8)|(1), // Channel 5 is 1!
    TCC0_CH6 = (0<<8)|(2), // Channel 6 is 2!
    TCC0_CH7 = (0<<8)|(3), // Channel 7 is 3!
    TCC1_CH0 = (1<<8)|(0),
    TCC1_CH1 = (1<<8)|(1),
    TCC1_CH2 = (1<<8)|(0), // Channel 2 is 0!
    TCC1_CH3 = (1<<8)|(1), // Channel 3 is 1!
    TCC2_CH0 = (2<<8)|(0),
    TCC2_CH1 = (2<<8)|(1),
    TCC2_CH2 = (2<<8)|(0), // Channel 2 is 0!
    TCC2_CH3 = (2<<8)|(1), // Channel 3 is 1!
    TC3_CH0  = (3<<8)|(0),
    TC3_CH1  = (3<<8)|(1),
    TC4_CH0  = (4<<8)|(0),
    TC4_CH1  = (4<<8)|(1),
    TC5_CH0  = (5<<8)|(0),
    TC5_CH1  = (5<<8)|(1),
#if defined __SAMD21J18A__
    TC6_CH0  = (6<<8)|(0),
    TC6_CH1  = (6<<8)|(1),
    TC7_CH0  = (7<<8)|(0),
    TC7_CH1  = (7<<8)|(1),
#endif // __SAMD21J18A__
} ETCChannel ;

// Definitions for PWM channels
typedef enum _EPWMChannel
{
    NOT_ON_PWM=-1,
    PWM0_CH0=TCC0_CH0,
    PWM0_CH1=TCC0_CH1,
    PWM0_CH2=TCC0_CH2,
    PWM0_CH3=TCC0_CH3,
    PWM0_CH4=TCC0_CH4,
    PWM0_CH5=TCC0_CH5,
    PWM0_CH6=TCC0_CH6,
    PWM0_CH7=TCC0_CH7,
    PWM1_CH0=TCC1_CH0,
    PWM1_CH1=TCC1_CH1,
    PWM1_CH2=TCC1_CH2,
    PWM1_CH3=TCC1_CH3,
    PWM2_CH0=TCC2_CH0,
    PWM2_CH1=TCC2_CH1,
    PWM2_CH2=TCC2_CH2,
    PWM2_CH3=TCC2_CH3,
    PWM3_CH0=TC3_CH0,
    PWM3_CH1=TC3_CH1,
    PWM4_CH0=TC4_CH0,
    PWM4_CH1=TC4_CH1,
    PWM5_CH0=TC5_CH0,
    PWM5_CH1=TC5_CH1,
#if defined __SAMD21J18A__
    PWM6_CH0=TC6_CH0,
    PWM6_CH1=TC6_CH1,
    PWM7_CH0=TC7_CH0,
    PWM7_CH1=TC7_CH1,
#endif // __SAMD21J18A__
} EPWMChannel ;

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
    EAnalogChannel  ulADCChannelNumber ; /* ADC Channel number in the SAM device */
    EPWMChannel     ulPWMChannel ;
    ETCChannel      ulTCChannel ;
    EExt_Interrupts ulExtInt ;
} PinDescription ;

#ifdef __cplusplus
extern "C" {
#endif

int pinPeripheral(uint32_t ulPin, EPioType ulPeripheral);

void pinMode(uint32_t ulPin, uint32_t ulMode);

void digitalWrite(uint32_t ulPin, uint32_t ulVal);

extern PinDescription g_APinDescription[];

#ifdef __cplusplus
}
#endif

#endif
