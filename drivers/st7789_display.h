#ifndef ST7789_DISPLAY_H
#define ST7789_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

// ST7789 Commands
#define ST7789_SWRESET    0x01
#define ST7789_SLPOUT     0x11
#define ST7789_NORON      0x13
#define ST7789_INVOFF     0x20
#define ST7789_INVON      0x21
#define ST7789_DISPOFF    0x28
#define ST7789_DISPON     0x29
#define ST7789_CASET      0x2A
#define ST7789_RASET      0x2B
#define ST7789_RAMWR      0x2C
#define ST7789_COLMOD     0x3A
#define ST7789_MADCTL     0x36

// Pin definitions based on your schematic
#define ST7789_SPI_INSTANCE  0
#define ST7789_SCK_PIN       29
#define ST7789_MOSI_PIN      30
#define ST7789_CS_PIN        28
#define ST7789_DC_PIN        31
#define ST7789_RST_PIN       32

// Display dimensions
#define ST7789_WIDTH         240
#define ST7789_HEIGHT        240

// SPI instance
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(ST7789_SPI_INSTANCE);

/**@brief Initialize ST7789 display
 */
void st7789_init(void);

/**@brief Send command to ST7789
 */
void st7789_send_command(uint8_t cmd);

/**@brief Send data to ST7789
 */
void st7789_send_data(uint8_t data);

/**@brief Set drawing window
 */
void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**@brief Draw bitmap to display
 */
void st7789_draw_bitmap(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t* bitmap);

/**@brief Fill screen with color
 */
void st7789_fill_screen(uint16_t color);

// Implementation
static void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
    // SPI transfer complete
}

void st7789_init(void)
{
    ret_code_t err_code;
    
    // Configure SPI
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = ST7789_CS_PIN;
    spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.mosi_pin = ST7789_MOSI_PIN;
    spi_config.sck_pin  = ST7789_SCK_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
    
    err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
    
    // Configure DC and RST pins
    nrf_gpio_cfg_output(ST7789_DC_PIN);
    nrf_gpio_cfg_output(ST7789_RST_PIN);
    
    // Reset display
    nrf_gpio_pin_clear(ST7789_RST_PIN);
    nrf_delay_ms(100);
    nrf_gpio_pin_set(ST7789_RST_PIN);
    nrf_delay_ms(100);
    
    // Initialize display
    st7789_send_command(ST7789_SWRESET);
    nrf_delay_ms(150);
    
    st7789_send_command(ST7789_SLPOUT);
    nrf_delay_ms(10);
    
    st7789_send_command(ST7789_COLMOD);
    st7789_send_data(0x55); // 16-bit color
    
    st7789_send_command(ST7789_MADCTL);
    st7789_send_data(0x00);
    
    st7789_send_command(ST7789_CASET);
    st7789_send_data(0x00);
    st7789_send_data(0x00);
    st7789_send_data(0x00);
    st7789_send_data(0xEF); // 239
    
    st7789_send_command(ST7789_RASET);
    st7789_send_data(0x00);
    st7789_send_data(0x00);
    st7789_send_data(0x00);
    st7789_send_data(0xEF); // 239
    
    st7789_send_command(ST7789_INVON);
    st7789_send_command(ST7789_NORON);
    st7789_send_command(ST7789_DISPON);
    nrf_delay_ms(10);
    
    // Clear screen
    st7789_fill_screen(0x0000); // Black
}

void st7789_send_command(uint8_t cmd)
{
    nrf_gpio_pin_clear(ST7789_DC_PIN); // Command mode
    nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
}

void st7789_send_data(uint8_t data)
{
    nrf_gpio_pin_set(ST7789_DC_PIN); // Data mode
    nrf_drv_spi_transfer(&spi, &data, 1, NULL, 0);
}

void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    st7789_send_command(ST7789_CASET);
    st7789_send_data(x0 >> 8);
    st7789_send_data(x0 & 0xFF);
    st7789_send_data(x1 >> 8);
    st7789_send_data(x1 & 0xFF);
    
    st7789_send_command(ST7789_RASET);
    st7789_send_data(y0 >> 8);
    st7789_send_data(y0 & 0xFF);
    st7789_send_data(y1 >> 8);
    st7789_send_data(y1 & 0xFF);
}

void st7789_draw_bitmap(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t* bitmap)
{
    st7789_set_window(x0, y0, x1, y1);
    st7789_send_command(ST7789_RAMWR);
    
    nrf_gpio_pin_set(ST7789_DC_PIN); // Data mode
    
    uint32_t size = (x1 - x0 + 1) * (y1 - y0 + 1) * 2; // 16-bit per pixel
    nrf_drv_spi_transfer(&spi, (uint8_t*)bitmap, size, NULL, 0);
}

void st7789_fill_screen(uint16_t color)
{
    st7789_set_window(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    st7789_send_command(ST7789_RAMWR);
    
    nrf_gpio_pin_set(ST7789_DC_PIN); // Data mode
    
    for (uint32_t i = 0; i < ST7789_WIDTH * ST7789_HEIGHT; i++) {
        uint8_t color_bytes[2] = {color >> 8, color & 0xFF};
        nrf_drv_spi_transfer(&spi, color_bytes, 2, NULL, 0);
    }
}

#endif // ST7789_DISPLAY_H
