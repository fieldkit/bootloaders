#include <stdarg.h>

#include <Arduino.h>
#include <WiFi101.h>
#include <SerialFlash.h>

#include "secrets.h"

static constexpr uint8_t WIFI_PIN_CS = 7u;
static constexpr uint8_t WIFI_PIN_IRQ = 16u;
static constexpr uint8_t WIFI_PIN_RST = 15u;
static constexpr uint8_t WIFI_PIN_EN = 38u;
static constexpr uint8_t FLASH_PIN_CS = 26u;
static constexpr uint8_t SD_PIN_CS = 12u;
static constexpr uint8_t RFM95_PIN_CS = 5u;

static void debugf(const char *f, ...);

constexpr size_t DEBUG_LINE_MAX = 256;

static void debugf(const char *f, ...) {
    char buffer[DEBUG_LINE_MAX];
    va_list args;

    va_start(args, f);
    vsnprintf(buffer, DEBUG_LINE_MAX, f, args);
    va_end(args);

    Serial5.print(buffer);
}

static void leds(uint8_t value) {
    digitalWrite(13, value);
    digitalWrite(A3, value);
    digitalWrite(A4, value);
    digitalWrite(A5, value);
}

static void panic(const char *message) {
    Serial5.println(message);
    
    while (true) {
        delay(100);
        leds(HIGH);
        delay(100);
        leds(LOW);
    }
}

static const char *getWifiStatus(uint8_t status) {
    switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    case WL_AP_LISTENING: return "WL_AP_LISTENING";
    case WL_AP_CONNECTED: return "WL_AP_CONNECTED";
    case WL_AP_FAILED: return "WL_AP_FAILED";
    case WL_PROVISIONING: return "WL_PROVISIONING";
    case WL_PROVISIONING_FAILED: return "WL_PROVISIONING_FAILED";
    default: return "Unknown";
    }
}

void upload(size_t length, size_t buffer_size) {
    WiFiClient wcl;

    wcl.stop();

    constexpr const char *server = "192.168.5.148";

    if (wcl.connect(server, 8080)) {
        Serial.println("Connecting...");
        wcl.println("POST /data.bin HTTP/1.1");
        wcl.print("Host: "); wcl.println(server);
        wcl.println("Content-Type: application/octet");
        wcl.print("Content-Length: "); wcl.println(length);
        wcl.println("User-Agent: ArduinoWiFi/1.1");
        wcl.println("Connection: close");
        wcl.println();

        Serial5.println("Connected...");

        auto uploadStarted = millis();
        auto lastStatus = 0;
        auto uploaded = 0;
        auto lastByte = 0;
        auto totalTimeInWrite = 0;
        auto timeInWrite = 0;
        uint8_t data[buffer_size];
        memset(data, 0xdf, sizeof(data));

        while (uploaded < length) {
            auto writeStarted = millis();
            auto wrote = wcl.write(data, sizeof(data));
            auto writeEnded = millis();
            if (wrote < 0) {
                debugf("Failed to write!\n\r");
                break;
            }
            if (wcl.getWriteError()) {
                debugf("Write Error! (%d)\n\r", wcl.getWriteError());
                break;
            }

            if (wrote > 0) {
                lastByte = millis();
            }

            totalTimeInWrite += writeEnded - writeStarted;
            timeInWrite += writeEnded - writeStarted;

            if (false && writeEnded - writeStarted > 1000) {
                debugf("Long call to write: %dms\n\r", writeEnded - writeStarted);
            }

            uploaded += wrote;
            if (millis() - lastStatus > 1000 || uploaded == length) {
                auto elapsed = (millis() - uploadStarted) / 1000.0f;
                auto complete = (uploaded / (float)length) * 100.0f;
                auto speed = ((uploaded / 1024.f) / (float)elapsed);
                auto rssi = WiFi.RSSI();
                debugf("Upload: %d/%d speed = %fkb/s complete = %f (%d) (tiw = %dms) (ttiw = %dms) (rssi = %d)\n\r", uploaded, length, speed, complete, buffer_size, timeInWrite, totalTimeInWrite, rssi);
                lastStatus = millis();

                timeInWrite = 0;
            }
        }

        Serial5.println("Stopping");
        wcl.stop();
    }
    else {
        Serial5.println("Connection failed");
    }
}

static void download() {
    WiFiClient wcl;

    wcl.stop();

    constexpr const char *server = "192.168.5.148";

    if (wcl.connect(server, 8080)) {
        Serial5.println("Connecting...");

        wcl.println("GET /data.bin HTTP/1.1");
        wcl.print("Host: "); wcl.println(server);
        wcl.println("User-Agent: ArduinoWiFi/1.1");
        wcl.println("Connection: close");
        wcl.println();
        wcl.flush();

        delay(100);

        Serial5.println("Downloading...");

        auto started = millis();
       
        while (true) {
            delay(10);

            while (wcl.available()) {
                Serial5.println("Read");
                char c = wcl.read();
                Serial5.write(c);
            }

            if (millis() - started > 10000) {
                Serial5.println("Fail");
                break;
            }

            #if 0
            if (wcl.available() > 0) {
                uint8_t buffer[512];

                Serial5.println("Data");

                if (wcl.read(buffer, sizeof(buffer)) > 0) {
                    Serial5.println("Data");
                }
            }
            #endif
        }

        Serial5.println("Stopping...");

        wcl.stop();

        Serial5.println("Done!");
    }
    else {
        Serial5.println("Connection failed");
    }
}

void setup() {
    Serial5.begin(115200);

    pinMode(13, OUTPUT);
    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    pinMode(FLASH_PIN_CS, OUTPUT);
    pinMode(SD_PIN_CS, OUTPUT);
    pinMode(WIFI_PIN_CS, OUTPUT);
    pinMode(RFM95_PIN_CS, OUTPUT);

    digitalWrite(FLASH_PIN_CS, HIGH);
    digitalWrite(SD_PIN_CS, HIGH);
    digitalWrite(WIFI_PIN_CS, HIGH);
    digitalWrite(RFM95_PIN_CS, HIGH);

    if (!SerialFlash.begin(FLASH_PIN_CS)) {
        panic("No serial flash");
    }

    WiFi.setPins(WIFI_PIN_CS, WIFI_PIN_IRQ, WIFI_PIN_RST, WIFI_PIN_EN);

    if (WiFi.status() == WL_NO_SHIELD) {
        panic("No wifi");
    }

    auto fv = WiFi.firmwareVersion();
    Serial5.print("Version: ");
    Serial5.println(fv);

    Serial5.println("Connecting...");

    WiFi.begin(WifiSsid, WifiPassword);

    Serial5.println("Ready!");

    auto statusAt = millis();
    auto done = false;

    while (true) {
        if (millis() - statusAt > 1000) {
            Serial5.print(getWifiStatus(WiFi.status()));
            Serial5.println("");
            statusAt = millis();
        }

        if (WiFi.status() == WL_CONNECTED && !done) {
            delay(1000);

            upload(1024 * 1024, 1024);

            download();
            done = true;
        }
    }
}

void loop() {
}
