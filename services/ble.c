#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble.h"
#include "ble_gap.h"
#include "nrf_ble_gatt.h"
#include "app_error.h"

#define DEVICE_NAME "Pinetime"
#define APP_BLE_CONN_CFG_TAG 1

static ble_gap_conn_sec_mode_t sec_mode;
static nrf_ble_gatt_t m_gatt;
static ble_advertising_t m_advertising;

void ble_stack_init(void) {
    ret_code_t err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}

void gatt_init(void) {
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

void advertising_start(void) {
    ble_gap_conn_sec_mode_set_open(&sec_mode);

    ble_gap_conn_params_t gap_conn_params = {0};
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = 0;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(4000, UNIT_10_MS);

    sd_ble_gap_device_name_set(&sec_mode,
                               (const uint8_t *)DEVICE_NAME,
                               strlen(DEVICE_NAME));

    ble_advertising_init_t init = {0};
    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = 64;
    init.config.ble_adv_fast_timeout = 30;
    init.evt_handler = NULL;

    ble_advertising_init(&m_advertising, &init);
    ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
}
