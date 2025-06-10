#include "lcd_st7789.h"
#include "lvgl.h"

void lcd_init(void) {
    // Init SPI + ST7789 sequence
}

void lcd_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * color_p) {
    // Write framebuffer to ST7789
    lv_disp_flush_ready(disp_drv);
}
