#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define PIN_SPI_MISO         (22u)
#define PIN_SPI_MOSI         (23u)
#define PIN_SPI_SCK          (24u)
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_0

#define WIFI_PIN_CS          (7u)
#define RFM95_PIN_CS         (5u)
#define SD_PIN_CS            (12u)
#define FLASH_PIN            (26u)
#define PERIPH_ENABLE_PIN    (25u)

void platform_setup();

void board_initialize(void);

void busy_delay(uint32_t ms);

void delay(uint32_t ms);

uint32_t millis();

void platform_default_sys_tick();

#endif // _PLATOFMR_H_
