/*  nrf52840_st7789_lvgl9.c
 *
 *  Works with LVGL 9.3 and the built-in ST7789 helper driver.
 *  Depends on: lvgl (>=9.3), nrfx (or Nordic nRF5 SDK), sdk_config.h with
 *              NRFX_SPIM_ENABLED == 1 and NRFX_SPIM3_ENABLED == 1
 *  Author: 2025-06-11
 */
#include "lvgl.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include <string.h>
#include <stdbool.h>

#define ST7789_PIN_BL    NRF_GPIO_PIN_MAP(1, 4)
static void bl_ctrl(bool on)         { nrf_gpio_pin_write(ST7789_PIN_BL,  on); }

#define ST7789_HOR_RES        240
#define ST7789_VER_RES        240

#define PIN_LCD_DC NRF_GPIO_PIN_MAP(0, 6 )
#define PIN_SPI_SCK NRF_GPIO_PIN_MAP(1, 9 )
#define PIN_SPI_MOSI NRF_GPIO_PIN_MAP(0, 12)
#define PIN_SPI_MISO 0xFF

// ST7789 specific commands used in init
#define ST7789_NOP 0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID 0x04
#define ST7789_RDDST 0x09

#define ST7789_RDDPM 0x0A       // Read display power mode
#define ST7789_RDD_MADCTL 0x0B  // Read display MADCTL
#define ST7789_RDD_COLMOD 0x0C  // Read display pixel format
#define ST7789_RDDIM 0x0D       // Read display image mode
#define ST7789_RDDSM 0x0E       // Read display signal mode
#define ST7789_RDDSR 0x0F       // Read display self-diagnostic result (ST7789V)

#define ST7789_SLPIN 0x10
#define ST7789_SLPOUT 0x11
#define ST7789_PTLON 0x12
#define ST7789_NORON 0x13

#define ST7789_INVOFF 0x20
#define ST7789_INVON 0x21
#define ST7789_GAMSET 0x26  // Gamma set
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_RGBSET 0x2D  // Color setting for 4096, 64K and 262K colors
#define ST7789_RAMRD 0x2E

#define ST7789_PTLAR 0x30
#define ST7789_VSCRDEF 0x33   // Vertical scrolling definition (ST7789V)
#define ST7789_TEOFF 0x34     // Tearing effect line off
#define ST7789_TEON 0x35      // Tearing effect line on
#define ST7789_MADCTL 0x36    // Memory data access control
#define ST7789_VSCRSADD 0x37  // Vertical screoll address
#define ST7789_IDMOFF 0x38    // Idle mode off
#define ST7789_IDMON 0x39     // Idle mode on
#define ST7789_RAMWRC 0x3C    // Memory write continue (ST7789V)
#define ST7789_RAMRDC 0x3E    // Memory read continue (ST7789V)
#define ST7789_COLMOD 0x3A

#define ST7789_RAMCTRL 0xB0    // RAM control
#define ST7789_RGBCTRL 0xB1    // RGB control
#define ST7789_PORCTRL 0xB2    // Porch control
#define ST7789_FRCTRL1 0xB3    // Frame rate control
#define ST7789_PARCTRL 0xB5    // Partial mode control
#define ST7789_GCTRL 0xB7      // Gate control
#define ST7789_GTADJ 0xB8      // Gate on timing adjustment
#define ST7789_DGMEN 0xBA      // Digital gamma enable
#define ST7789_VCOMS 0xBB      // VCOMS setting
#define ST7789_LCMCTRL 0xC0    // LCM control
#define ST7789_IDSET 0xC1      // ID setting
#define ST7789_VDVVRHEN 0xC2   // VDV and VRH command enable
#define ST7789_VRHS 0xC3       // VRH set
#define ST7789_VDVSET 0xC4     // VDV setting
#define ST7789_VCMOFSET 0xC5   // VCOMS offset set
#define ST7789_FRCTR2 0xC6     // FR Control 2
#define ST7789_CABCCTRL 0xC7   // CABC control
#define ST7789_REGSEL1 0xC8    // Register value section 1
#define ST7789_REGSEL2 0xCA    // Register value section 2
#define ST7789_PWMFRSEL 0xCC   // PWM frequency selection
#define ST7789_PWCTRL1 0xD0    // Power control 1
#define ST7789_VAPVANEN 0xD2   // Enable VAP/VAN signal output
#define ST7789_CMD2EN 0xDF     // Command 2 enable
#define ST7789_PVGAMCTRL 0xE0  // Positive voltage gamma control
#define ST7789_NVGAMCTRL 0xE1  // Negative voltage gamma control
#define ST7789_DGMLUTR 0xE2    // Digital gamma look-up table for red
#define ST7789_DGMLUTB 0xE3    // Digital gamma look-up table for blue
#define ST7789_GATECTRL 0xE4   // Gate control
#define ST7789_SPI2EN 0xE7     // SPI2 enable
#define ST7789_PWCTRL2 0xE8    // Power control 2
#define ST7789_EQCTRL 0xE9     // Equalize time control
#define ST7789_PROMCTRL 0xEC   // Program control
#define ST7789_PROMEN 0xFA     // Program mode enable
#define ST7789_NVMSET 0xFC     // NVM setting
#define ST7789_PROMACT 0xFE    // Program action

#define TFT_MAD_RGB 0x00
#define TFT_MAD_BGR 0x08
#define TFT_INVOFF 0x20
#define TFT_INVON 0x21

#define TFT_MAD_COLOR_ORDER TFT_MAD_BGR

#define PIN_LCD_CSN NRF_GPIO_PIN_MAP(0, 8 )
#define PIN_LCD_RST NRF_GPIO_PIN_MAP(0, 11)
#define PIN_LCD_DC NRF_GPIO_PIN_MAP(0, 6 )

void init_fast_spi() {

  NRF_SPIM3->ENABLE = SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos;
  NRF_SPIM3->PSEL.SCK = PIN_SPI_SCK;;
  NRF_SPIM3->PSEL.MOSI = PIN_SPI_MOSI;
  NRF_SPIM3->PSEL.MISO = PIN_SPI_MISO;

  NRF_SPIM3->CONFIG = (SPIM_CONFIG_ORDER_MsbFirst << SPIM_CONFIG_ORDER_Pos) |
    (SPIM_CONFIG_CPOL_ActiveLow << SPIM_CONFIG_CPOL_Pos) |
    (SPIM_CONFIG_CPHA_Trailing << SPIM_CONFIG_CPHA_Pos);

  NRF_SPIM3->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32 << SPIM_FREQUENCY_FREQUENCY_Pos;

  // setup CMD pin
  NRF_GPIOTE->CONFIG[1] = GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos |
    GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos |
    PIN_LCD_DC << GPIOTE_CONFIG_PSEL_Pos |
    GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos;

  NRF_SPIM3->ENABLE = SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos;

}

void write_fast_spi(const uint8_t* ptr, uint32_t len, bool mode) {

  if (mode)
    NRF_GPIOTE->TASKS_SET[1] = 1;
  else
    NRF_GPIOTE->TASKS_CLR[1] = 1;

  int v2 = 0;
  do
  {
    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->EVENTS_ENDTX = 0;
    NRF_SPIM3->TXD.PTR = (uint32_t)ptr + v2;
    if (len <= 0xFF)
    {
      NRF_SPIM3->TXD.MAXCNT = len;
      v2 += len;
      len = 0;
    }
    else
    {
      NRF_SPIM3->TXD.MAXCNT = 255;
      v2 += 255;
      len -= 255;
    }
    NRF_SPIM3->RXD.PTR = 0;
    NRF_SPIM3->RXD.MAXCNT = 0;
    NRF_SPIM3->TASKS_START = 1;
    while (NRF_SPIM3->EVENTS_END == 0);
    NRF_SPIM3->EVENTS_END = 0;
  } while (len);

  NRF_SPIM3->TASKS_STOP = 1;
  while (NRF_SPIM3->EVENTS_STOPPED == 0) __NOP();

}

void write_command(uint8_t value) {
    write_fast_spi(&value, 1, false);
}
void write_data(uint8_t value) {
    write_fast_spi(&value, 1, true);
}
void write_buffer(const uint8_t* ptr, uint32_t len) {
    write_fast_spi(ptr, len, true);
}

void set_addr_display(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    uint8_t temp[4];
    write_command(ST7789_CASET);
    temp[0] = (x >> 8);
    temp[1] = x;
    temp[2] = ((x + w - 1) >> 8);
    temp[3] = (x + w - 1);
    write_buffer(temp, 4);
    write_command(ST7789_RASET);
    temp[0] = (y >> 8);
    temp[1] = y;
    temp[2] = ((y + h - 1) >> 8);
    temp[3] = ((y + h - 1) & 0xFF);
    write_buffer(temp, 4);
    write_command(ST7789_RAMWR);
}

void init_display(void) {

    // start the SPI library:
    init_fast_spi();
    nrf_gpio_pin_set(PIN_LCD_CSN);
    nrf_gpio_cfg_output(PIN_LCD_CSN);

    nrf_gpio_pin_set(PIN_LCD_DC);
    nrf_gpio_cfg_output(PIN_LCD_DC);

    nrf_gpio_pin_set(PIN_LCD_RST);
    nrf_gpio_cfg_output(PIN_LCD_RST);

    nrf_gpio_pin_clear(PIN_LCD_CSN);    

    write_command(0x00);  // Put SPI bus in known state for TFT with CS tied low    
    nrf_gpio_pin_set(PIN_LCD_RST);
    nrf_delay_ms(5);    
    nrf_gpio_pin_clear(PIN_LCD_RST);
    nrf_delay_ms(20);    
    nrf_gpio_pin_set(PIN_LCD_RST);

    nrf_delay_ms(150);  // Wait for reset to complete

    write_command(ST7789_SWRESET);
    nrf_delay_ms(150);  // Wait for reset to complete

    write_command(ST7789_SLPOUT);  // Sleep out
    nrf_delay_ms(120);

    write_command(ST7789_NORON);  // Normal display mode on

    //------------------------------display and color format setting--------------------------------//
    write_command(ST7789_MADCTL);
    //write_data(0x00);
    write_data(TFT_MAD_COLOR_ORDER);

    // JLX240 display datasheet
    /*write_command(0xB6);
    write_data(0x0A);
    write_data(0x82);*/

    write_command(ST7789_RAMCTRL);
    write_data(0x00);
    write_data(0xE0);  // 5 to 6 bit conversion: r0 = r5, b0 = b5

    write_command(ST7789_COLMOD);
    write_data(0x55);  //16bit/pixel
    //write_data(0x53); // 12bit/pixel
    nrf_delay_ms(10);

    //--------------------------------ST7789V Frame rate setting----------------------------------//
    write_command(ST7789_PORCTRL);
    write_data(0x0c);
    write_data(0x0c);
    write_data(0x00);
    write_data(0x33);
    write_data(0x33);

    write_command(ST7789_GCTRL);  // Voltages: VGH / VGL
    write_data(0x35);

    //---------------------------------ST7789V Power setting--------------------------------------//
    write_command(ST7789_VCOMS);
    write_data(0x28);  // JLX240 display datasheet

    write_command(ST7789_LCMCTRL);
    write_data(0x0C);

    write_command(ST7789_VDVVRHEN);
    write_data(0x01);
    write_data(0xFF);

    write_command(ST7789_VRHS);  // voltage VRHS
    write_data(0x10);

    write_command(ST7789_VDVSET);
    write_data(0x20);

    write_command(ST7789_FRCTR2);
    write_data(0x0f);

    write_command(ST7789_PWCTRL1);
    write_data(0xa4);
    write_data(0xa1);

    //--------------------------------ST7789V gamma setting---------------------------------------//
    write_command(ST7789_PVGAMCTRL);
    write_data(0xd0);
    write_data(0x00);
    write_data(0x02);
    write_data(0x07);
    write_data(0x0a);
    write_data(0x28);
    write_data(0x32);
    write_data(0x44);
    write_data(0x42);
    write_data(0x06);
    write_data(0x0e);
    write_data(0x12);
    write_data(0x14);
    write_data(0x17);

    write_command(ST7789_NVGAMCTRL);
    write_data(0xd0);
    write_data(0x00);
    write_data(0x02);
    write_data(0x07);
    write_data(0x0a);
    write_data(0x28);
    write_data(0x31);
    write_data(0x54);
    write_data(0x47);
    write_data(0x0e);
    write_data(0x1c);
    write_data(0x17);
    write_data(0x1b);
    write_data(0x1e);

    //write_command(ST7789_INVON);

    write_command(ST7789_CASET);  // Column address set
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0xE5);  // 239

    write_command(ST7789_RASET);  // Row address set
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0xE5);  // 239

    nrf_delay_ms(120);

    write_command(ST7789_DISPON);  //Display on
    nrf_delay_ms(120);

    write_command(TFT_INVON);

}

void draw_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* data) {
    set_addr_display(x, y, w, h);
    //write_command(ST7789_RAMWR);
    write_buffer(data, (w * h) * 2);
}


void write_display(uint8_t* data, uint16_t len) {    
    //write_command(ST7789_RAMWR);
    write_buffer(data, len);
}

#define RGB2COLOR(r, g, b) ((((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))

#define BCK_COLOR RGB2COLOR(0x00, 0x00, 0xFF)

void display_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  uint16_t i;

  uint8_t linebuffer[2 * 240];
  uint16_t bp = 0;

  set_addr_display(x1, y1, x2, y2);

  for (i = y1; i < y2; i++) {
    uint8_t rl = (x2 - x1);
    while (rl) {
      linebuffer[bp] = color >> 8;
      linebuffer[bp + 1] = color & 0xff;
      bp += 2;
      rl -= 1;

      if (bp >= sizeof(linebuffer)) {
        write_display(linebuffer, sizeof(linebuffer));
        bp = 0;
      }
    }
  }
}


void st7789_nrf52_init(void)
{
    init_display();
    display_fill(0, 0, 240, 240, BCK_COLOR);

    NRFX_DELAY_US(100);
}

//#define DRAW_BUF_SIZE (ST7789_HOR_RES * ST7789_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
//uint32_t draw_buf[DRAW_BUF_SIZE / 4];

void display(void)
{

    st7789_nrf52_init();
    // Initialize backlight control pin
    nrf_gpio_cfg_output(ST7789_PIN_BL);
    bl_ctrl(false);

    //lv_init();                              

    
}
