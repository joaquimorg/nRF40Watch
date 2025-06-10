#include "lvgl.h"
#include "gui.h"

static lv_obj_t *label;

void gui_init(void) {
    lv_obj_clean(lv_scr_act());

    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Welcome");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
