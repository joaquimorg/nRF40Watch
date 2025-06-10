#ifndef TOUCH_CST816S_H
#define TOUCH_CST816S_H

#include "lvgl.h"

void touch_init(void);
void touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data);

#endif // TOUCH_CST816S_H
