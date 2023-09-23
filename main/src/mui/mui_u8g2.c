#include "mui_u8g2.h"

/*
 * u8g2 Demo Application running on Nordic Semiconductors nRF52 DK with nRF SDK 17.02 and:
 *
 * - TWIM and a Waveshield SH1106 OLED 128x64 display
 * or
 * - SPIM and a Waveshield SSD1327 OLED 128x128 16 grey level display
 * that can be found on Ebay/Amazon for few EUR.
 *
 * Created 20200102 by @daubsi, based on the great u8g2 library
 * https://github.com/olikraus/u8g2 Probably not the cleanest code but it works :-D
 *
 * TWI: Set up with i2c/TWIM in 400 kHz (100 kHz works equally well) in non-blocking mode
 * SPI: Setup up in SPIM with 4 MHz in non-blocking mode
 *
 * Wiring TWI:
 * Connect VCC and GND of the display with VDD and GND on the nRF52 DK
 * Connect SCL to pin 27 ("P0.27") and SDA to pin 26 ("P0.26") on the nRF52 DK
 *
 * Wiring SPI:
 * Connect VCC and GND of the diplay with VDD and GND on the NRF52 DK
 * Connect CLK (yellow wire) to pin 27 ("P0.27") and DIN (blue wire) to pin 26 ("P0.26")
 * on the nRF52 DK Connect CS (orange wire) to pin 12 ("P0.12") and DC (green wire) to pin
 * 11 ("P0.11") on the nRF52 DK Connect Reset (white wire) to pin 31 ("P0.31") on the
 * nRF52 DK
 *
 * Settings in sdk_config.h
 *
 * TWI:
 * #define NRFX_TWIM_ENABLED 1
 * #define NRFX_TWIM0_ENABLED 1
 * #define NRFX_TWIM1_ENABLED 0
 * #define NRFX_TWIM_DEFAULT_CONFIG_FREQUENCY 104857600
 * #define NRFX_TWIM_DEFAULT_CONFIG_HOLD_BUS_UNINIT 0
 * #define TWI_ENABLED 1
 * #define TWI_DEFAULT_CONFIG_FREQUENCY 104857600
 * #define TWI_DEFAULT_CONFIG_CLR_BUS_INIT 0
 * #define TWI_DEFAULT_CONFIG_HOLD_BUS_UNINIT 0
 * #define TWI0_ENABLED 1
 * #define TWI0_USE_EASY_DMA 1
 * #define TWI1_ENABLED 0
 * #define TWI1_USE_EASY_DMA 1
 *
 * SWI:
 * #define NRFX_SPIM_ENABLED 1
 * #define NRFX_SPIM0_ENABLED 0
 * #define NRFX_SPIM1_ENABLED 1
 * #define NRFX_SPIM_EXTENDED_ENABLED 0 // Only available on NRF52840 on SPIM3
 * #define NRFX_SPIM_MISO_PULL_CFG 1
 * #define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 6
 *
 * Probably most of those configs are not needed apart from TWI_ENABLED and TWI0_ENABLED
 * but I kept them there
 *
 * When compiling for SPI (== #define USE_SPI 1) and TWI (== #define USE_TWI 1) at the
 * same time, make sure, that you use different HW resources, e.g. TWI_INSTANCE_ID = 0 and
 * SPI_INSTANCE_ID = 1 (and the corresponding TWI0_ENABLED 1, SPIM1_ENABLED 1 in
 * sdk_config.h Otherwise there vill be linker conflicts when using the same id.
 * https://devzone.nordicsemi.com/f/nordic-q-a/35182/irq-handler-compile-error
 *
 * The u8g2 sources have to be unpacked and the csrc subfolder of that archive should be
 * placed in the main directory of this project in a folder u8g2.
 */

#include <stdio.h>

#include "u8g2.h"
#include "u8x8.h"
