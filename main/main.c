#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "app_timer.h"
#include "nrf_delay.h"

#include "lcd_st7789.h"
#include "touch_cst816s.h"
#include "backlight.h"
#include "gui.h"
#include "ble.h"
#include "time.h"

#include "lvgl.h"

#define APP_TIMER_PRESCALER 0
#define APP_TIMER_OP_QUEUE_SIZE 4


static void lv_tick_handler(void) {
    lv_tick_inc(1);
}

static void clock_init(void) {
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

static void timers_init(void) {
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    app_timer_start(app_timer_create_static(0, APP_TIMER_MODE_REPEATED, lv_tick_handler), APP_TIMER_TICKS(1), NULL);
}

int main(void) {
    clock_init();
    timers_init();
    nrf_pwr_mgmt_init();

    ble_stack_init();
    gatt_init();
    advertising_start();

    lcd_init();
    touch_init();
    backlight_init();

    lv_init();
    gui_init();

    while (true) {
        lv_task_handler();
        nrf_pwr_mgmt_run();
        nrf_delay_ms(5);
    }
}