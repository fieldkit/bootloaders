#include <stdarg.h>

#include <Arduino.h>

#include <phylum/backend.h>
#include <phylum/private.h>
#include <phylum/serial_flash_state_manager.h>
#include <phylum/serial_flash_fs.h>
#include <backends/arduino_serial_flash/arduino_serial_flash.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include <WiFi101.h>

#include "firmware_header.h"
#include "core_state.h"

#include "http_response_parser.h"
#include "http_response_writer.h"

#include "secrets.h"

static constexpr uint8_t WIFI_PIN_CS = 7u;
static constexpr uint8_t WIFI_PIN_IRQ = 16u;
static constexpr uint8_t WIFI_PIN_RST = 15u;
static constexpr uint8_t WIFI_PIN_EN = 38u;
static constexpr uint8_t FLASH_PIN_CS = 26u;
static constexpr uint8_t SD_PIN_CS = 12u;
static constexpr uint8_t RFM95_PIN_CS = 5u;

static void leds(uint8_t value);
static void panic(const char *message);
static void debugf(const char *f, ...);
static void debugln(const char *f, ...);
static const char *wifi_status_get(uint8_t status);

class FirmwareStorage {
private:
    phylum::NoopStorageBackendCallbacks callbacks_;
    phylum::ArduinoSerialFlashBackend backend_{ callbacks_ };
    phylum::SerialFlashAllocator allocator_{ backend_ };
    phylum::SerialFlashStateManager<CoreState> manager_{ backend_, allocator_ };
    phylum::AllocatedBlockedFile opened_;

public:
    bool initialize() {
        if (!backend_.initialize(FLASH_PIN_CS, 512)) {
            panic("Not availabl\ne");
            return false;
        }

        if (false) {
            debugln("Freeing...");
            allocator_.free_all_blocks();

            debugln("Creating...");
            if (!manager_.create()) {
                panic("Create failed\n");
                return false;
            }
        }

        if (!manager_.locate()) {
            panic("Not found\n");
            return false;
        }

        phylum::Files files(&backend_, &allocator_);
        phylum::UnusedBlockReclaimer reclaimer(&files, &manager_);
        auto &state = manager_.state();
        for (auto i = 0; i < (int32_t)FirmwareBank::NumberOfBanks; ++i) {
            auto addr = state.firmwares.banks[i];
            if (backend_.geometry().valid(addr)) {
                debugln("Walk (%lu:%lu)", addr.block, addr.position);
                reclaimer.walk(addr);
            }
            else {
                // NOTE: This fixes up some gibberish addresses I introduced in testing.
                state.firmwares.banks[i] = { };
            }
        }

        manager_.save();

        reclaimer.reclaim();

        debugln("Ready");
        return true;
    }

    phylum::BlockedFile &write() {
        opened_ = phylum::AllocatedBlockedFile(&backend_, phylum::OpenMode::Write, &allocator_, { });

        if (!opened_.format()) {
            panic("Unable to format file.");
        }

        return opened_;
    }

    bool update(FirmwareBank bank, const char *etag) {
        opened_.close();

        auto previousAddr = manager_.state().firmwares.banks[(int32_t)bank];

        manager_.state().firmwares.banks[(int32_t)bank] = opened_.beginning();

        if (!manager_.save()) {
            panic("Error saving block");
        }

        if (previousAddr.valid()) {
            phylum::AllocatedBlockedFile previousFile(&backend_, phylum::OpenMode::Write, &allocator_, previousAddr);
            if (previousFile.exists()) {
                previousFile.erase_all_blocks();
            }
        }

        auto loc = manager_.location();

        debugln("Bank %d: Saved firmware (%lu:%lu)", bank, loc.block, loc.sector);

        return true;
    }

    bool header(FirmwareBank bank, firmware_header_t *header) {
        auto addr = manager_.state().firmwares.banks[(int32_t)bank];
        if (!addr.valid()) {
            return false;
        }

        auto file = phylum::AllocatedBlockedFile(&backend_, phylum::OpenMode::Read, &allocator_, addr);
        if (!file.exists()) {
            return false;
        }
        if (file.read((uint8_t *)header, sizeof(firmware_header_t)) != sizeof(firmware_header_t)) {
            return false;
        }

        return true;
    }
};

static void download(FirmwareStorage &firmware) {
    const char *url = "http://api.fkdev.org/devices/0004a30b001d00ff/fk-core/firmware";

    fk::Url parsed(url);

    debugln("Reading existing file.");

    firmware_header_t header;
    firmware.header(FirmwareBank::CoreNew, &header);

    debugln("Existing: '%s'", header.etag);

    debugln("Opening new firmware file.");

    auto &file = firmware.write();

    debugln("GET %s", url);

    debugln("Connecting...");

    WiFiClient wcl;
    wcl.stop();
    if (wcl.connect(parsed.server, parsed.port)) {
        fk::OutgoingHttpHeaders headers{
            nullptr,
            "Version",
            "Build",
            "Device-Id",
            header.version == FIRMWARE_VERSION_INVALID ? nullptr : header.etag
        };
        fk::HttpHeadersWriter httpWriter(wcl);
        fk::HttpResponseParser httpParser;

        debugln("Connected!");

        httpWriter.writeHeaders(parsed, "GET", headers);

        auto started = millis();
        auto total = 0;

        while (wcl.connected() || wcl.available()) {
            delay(10);

            if (millis() - started > 10000) {
                debugln("Fail");
                break;
            }

            while (wcl.available()) {
                if (httpParser.reading_header()) {
                    httpParser.write(wcl.read());
                }
                else {
                    uint8_t buffer[512];

                    auto bytes = wcl.read(buffer, sizeof(buffer));
                    if (bytes > 0) {
                        if (httpParser.status_code() == 200) {
                            if (total == 0) {
                                firmware_header_t header;
                                header.version = 1;
                                header.time = 0;
                                header.size = httpParser.content_length();
                                strncpy(header.etag, httpParser.etag(), sizeof(header.etag) - 1);

                                debugln("Writing header ('%s') ('%s')", httpParser.etag(), header.etag);

                                auto headerBytes = file.write((uint8_t *)&header, sizeof(firmware_header_t));
                                if (headerBytes != sizeof(firmware_header_t)) {
                                    panic("Writing header failed.");
                                }

                                debugln("Downloading");
                            }

                            total += bytes;
                            file.write(buffer, bytes);
                        }
                    }
                }
            }
        }

        wcl.stop();

        debugln("Done! (%d) (%d bytes)", httpParser.status_code(), total);

        if (total > 0) {
            firmware.update(FirmwareBank::CoreNew, httpParser.etag());
            debugln("Waiting 5s before rebooting.");
            delay(5000);
            firmware_self_flash();
        }
        else {
            file.erase_all_blocks();
        }
    }
    else {
        debugln("Connection failed");
        file.erase_all_blocks();
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

    FirmwareStorage firmware;
    if (!firmware.initialize()) {
        panic("Failed to initialize firmwareStorage");
    }

    WiFi.setPins(WIFI_PIN_CS, WIFI_PIN_IRQ, WIFI_PIN_RST, WIFI_PIN_EN);

    if (WiFi.status() == WL_NO_SHIELD) {
        panic("No wifi");
    }

    auto fv = WiFi.firmwareVersion();
    debugln("Version: %s, connecting...", fv);

    WiFi.begin(WifiSsid, WifiPassword);

    debugln("Ready!");

    auto statusAt = millis();
    auto done = false;

    while (true) {
        if (millis() - statusAt > 1000) {
            debugln("%s", wifi_status_get(WiFi.status()));
            statusAt = millis();
        }

        if (WiFi.status() == WL_CONNECTED) {
            download(firmware);

            while (true) {
                delay(500);
            }
        }
    }
}

void loop() {
}

constexpr size_t DEBUG_LINE_MAX = 256;

static void debugln(const char *f, ...) {
    char buffer[DEBUG_LINE_MAX];
    va_list args;

    va_start(args, f);
    vsnprintf(buffer, DEBUG_LINE_MAX, f, args);
    va_end(args);

    Serial5.println(buffer);
}

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

static const char *wifi_status_get(uint8_t status) {
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
