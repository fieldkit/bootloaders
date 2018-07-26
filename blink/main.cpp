#include <Arduino.h>

static void leds(uint8_t value) {
    digitalWrite(13, value);
    digitalWrite(A3, value);
    digitalWrite(A4, value);
    digitalWrite(A5, value);
}

void setup() {
    Serial5.begin(115200);

    pinMode(13, OUTPUT);
    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    while (true) {
        leds(true);
        Serial5.print(".");
        delay(500);

        leds(false);
        Serial5.print(".");
        delay(500);
    }
}

void loop() {

}
