#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define PIN_SPI_MISO         (22u)
#define PIN_SPI_MOSI         (23u)
#define PIN_SPI_SCK          (24u)
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_0

typedef struct board_configuration_t {
    uint8_t periph_enable;
    uint8_t flash_cs;
    uint8_t sd_cs;
    uint8_t wifi_cs;
    uint8_t rfm95_cs;
} board_configuration_t;

#define FK_NUMBER_OF_POSSIBLE_BOARDS 4

extern board_configuration_t possible_boards[FK_NUMBER_OF_POSSIBLE_BOARDS];

#define INACTIVITY_TIMEOUT   (1000 * 60 * 2)

#ifdef __cplusplus
extern "C" {
#endif

void board_prepare(board_configuration_t *cfg);

void platform_setup();

void board_initialize(void);

void busy_delay(uint32_t ms);

void delay(uint32_t ms);

uint32_t millis();

void platform_default_sys_tick();

#ifdef __cplusplus
}
#endif

#endif // _PLATOFMR_H_
