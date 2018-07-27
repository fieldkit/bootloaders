#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define PIN_SPI_MISO         (22u)
#define PIN_SPI_MOSI         (23u)
#define PIN_SPI_SCK          (24u)
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_0
#define PIN_FLASH            (26u)

void platform_setup();

#endif // _PLATOFMR_H_
