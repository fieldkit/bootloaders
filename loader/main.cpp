#include <stdarg.h>

#include <Arduino.h>
#include <WiFi101.h>
#include <SerialFlash.h>

#include "http_response_parser.h"

#include "secrets.h"

#define FLASH_FIRMWARE_BANK_ADDRESS          (1572864)
#define FLASH_FIRMWARE_BANK_SIZE             (256 * 1024)
#define FLASH_FIRMWARE_BANK_1_ADDRESS        (1572864)
#define FLASH_FIRMWARE_BANK_2_ADDRESS        (1572864 + FLASH_FIRMWARE_BANK_SIZE)
#define FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS (FLASH_FIRMWARE_BANK_1_ADDRESS + FLASH_FIRMWARE_BANK_SIZE - sizeof(firmware_header_t))
#define FLASH_FIRMWARE_BANK_2_HEADER_ADDRESS (FLASH_FIRMWARE_BANK_2_ADDRESS + FLASH_FIRMWARE_BANK_SIZE - sizeof(firmware_header_t))

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
    debugf(message);
    debugf("\n");

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

static void flash_erase() {
    auto starting = FLASH_FIRMWARE_BANK_1_ADDRESS;
    auto block_size = SerialFlash.blockSize();
    auto address = starting;

    debugf("Erasing: %d -> %d\n", starting, starting + block_size * 8);

    for (auto i = 0; i < 8; ++i) {
        auto address = starting + i * block_size;
        SerialFlash.eraseBlock(address);
    }
}

typedef struct firmware_header_t {
    uint32_t version;
    uint32_t position;
    uint32_t size;
    uint8_t reserved[64 - (4 * 3)];
} firmware_header_t;

static void download() {
    fk::HttpResponseParser parser;
    WiFiClient wcl;

    constexpr const char *server = "192.168.0.141";

    uint32_t starting = FLASH_FIRMWARE_BANK_1_ADDRESS;
    uint32_t address = starting;

    wcl.stop();

    if (wcl.connect(server, 8080)) {
        debugf("Connecting...\n");

        wcl.println("GET /blink.bin HTTP/1.1");
        wcl.print("Host: "); wcl.println(server);
        wcl.println("User-Agent: ArduinoWiFi/1.1");
        wcl.println("Connection: close");
        wcl.println();
        wcl.flush();

        delay(100);

        debugf("Downloading...\n");

        auto started = millis();
        auto total = 0;

        while (wcl.connected() || wcl.available()) {
            delay(10);

            if (millis() - started > 10000) {
                debugf("Fail\n");
                break;
            }

            while (wcl.available()) {
                if (parser.reading_header()) {
                    parser.write(wcl.read());
                }
                else {
                    uint8_t buffer[512];

                    auto bytes = wcl.read(buffer, sizeof(buffer));
                    if (bytes > 0) {
                        SerialFlash.write(address, buffer, bytes);

                        total += bytes;
                        address += bytes;
                    }
                }
            }
        }

        firmware_header_t header = { 1, starting, parser.content_length() };
        SerialFlash.write(FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS, &header, sizeof(header));

        wcl.stop();

        debugf("Done! (%d bytes)\n", total);
    }
    else {
        debugf("Connection failed\n");
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
    debugf("Version: %s\n", fv);
    debugf("Connecting...\n");

    WiFi.begin(WifiSsid, WifiPassword);

    debugf("Ready!\n");

    auto statusAt = millis();
    auto done = false;

    while (true) {
        if (millis() - statusAt > 1000) {
            debugf("%s\n", getWifiStatus(WiFi.status()));
            statusAt = millis();
        }

        if (WiFi.status() == WL_CONNECTED) {
            flash_erase();
            download();
            debugf("Address: %d\n", FLASH_FIRMWARE_BANK_1_ADDRESS);
            while (true) {
                delay(500);
            }
        }
    }
}

void loop() {
}
