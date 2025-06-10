#ifndef LCD_ST7789_H
#define LCD_ST7789_H

void lcd_init(void);
void lcd_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * color_p);

#endif // LCD_ST7789_H
