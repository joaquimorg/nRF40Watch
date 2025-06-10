#include "touch_cst816s.h"
#include "lvgl.h"

void touch_init(void) {
    // Init I2C and CST816S
}

void touch_read_cb(lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    data->state = LV_INDEV_STATE_REL;
}
