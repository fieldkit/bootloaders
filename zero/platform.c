#include "wiring.h"
#include "platform.h"

void platform_setup() {
    pinMode(RFM95_PIN_CS, OUTPUT);
    pinMode(SD_PIN_CS, OUTPUT);
    pinMode(WIFI_PIN_CS, OUTPUT);
    pinMode(FLASH_PIN, OUTPUT);

    digitalWrite(RFM95_PIN_CS, HIGH);
    digitalWrite(SD_PIN_CS, HIGH);
    digitalWrite(WIFI_PIN_CS, HIGH);
    digitalWrite(FLASH_PIN, HIGH);
}
