#include "wiring.h"
#include "platform.h"

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

