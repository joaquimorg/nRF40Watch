#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
//#include "bsp_btn_ble.h"
#include "fds.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// LVGL includes
#include "lvgl.h"

// Custom includes
#include "st7789_display.h"
#include "step_counter.h"
#include "time_manager.h"
#include "notification_service.h"

#define DEVICE_NAME                     "nRF40Watch"
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN
#define APP_BLE_OBSERVER_PRIO           3
#define APP_BLE_CONN_CFG_TAG            1

#define APP_ADV_INTERVAL                64
#define APP_ADV_DURATION                18000
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)
#define SLAVE_LATENCY                   0
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)
#define MAX_CONN_PARAMS_UPDATE_COUNT    3

// Timer definitions
APP_TIMER_DEF(m_display_timer_id);
APP_TIMER_DEF(m_step_timer_id);
APP_TIMER_DEF(m_time_timer_id);

BLE_ADVERTISING_DEF(m_advertising);
NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;
static ble_uuid_t m_adv_uuids[]          =
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

// Smartwatch state
typedef struct {
    bool connected;
    uint32_t step_count;
    time_t current_time;
    char notification_text[256];
    bool has_notification;
} smartwatch_state_t;

static smartwatch_state_t m_watch_state = {0};

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[240 * 10];

// LVGL objects
static lv_obj_t * time_label;
static lv_obj_t * step_label;
static lv_obj_t * notification_label;
static lv_obj_t * bt_status_label;

/**@brief Function for assert macro callback.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling BLE events.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            m_watch_state.connected = true;
            nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            m_watch_state.connected = false;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

/**@brief Function for the SoftDevice initialization.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = NULL;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

/**@brief LVGL display flush callback.
 */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    st7789_draw_bitmap(area->x1, area->y1, area->x2, area->y2, (uint16_t*)color_p);
    lv_disp_flush_ready(disp_drv);
}

/**@brief Initialize LVGL display driver.
 */
static void lvgl_init(void)
{
    lv_init();
    
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 240 * 10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

/**@brief Create the main watch face UI.
 */
static void create_watch_ui(void)
{
    // Create main container
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    
    // Time display
    time_label = lv_label_create(scr);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_28, 0);
    lv_label_set_text(time_label, "00:00");
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 30);
    
    // Step counter
    step_label = lv_label_create(scr);
    lv_obj_set_style_text_color(step_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(step_label, &lv_font_montserrat_18, 0);
    lv_label_set_text(step_label, "Steps: 0");
    lv_obj_align(step_label, LV_ALIGN_CENTER, 0, 0);
    
    // Bluetooth status
    bt_status_label = lv_label_create(scr);
    lv_obj_set_style_text_color(bt_status_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(bt_status_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(bt_status_label, "BT: Disconnected");
    lv_obj_align(bt_status_label, LV_ALIGN_TOP_RIGHT, -10, 10);
    
    // Notification area
    notification_label = lv_label_create(scr);
    lv_obj_set_style_text_color(notification_label, lv_color_yellow(), 0);
    lv_obj_set_style_text_font(notification_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(notification_label, "");
    lv_obj_align(notification_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_label_set_long_mode(notification_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(notification_label, 200);
}

/**@brief Update the watch display.
 */
static void update_display(void)
{
    char time_str[32];
    char step_str[32];
    
    // Update time
    time_manager_get_time_string(time_str, sizeof(time_str));
    lv_label_set_text(time_label, time_str);
    
    // Update steps
    snprintf(step_str, sizeof(step_str), "Steps: %lu", m_watch_state.step_count);
    lv_label_set_text(step_label, step_str);
    
    // Update Bluetooth status
    if (m_watch_state.connected) {
        lv_label_set_text(bt_status_label, "BT: Connected");
        lv_obj_set_style_text_color(bt_status_label, lv_color_green(), 0);
    } else {
        lv_label_set_text(bt_status_label, "BT: Disconnected");
        lv_obj_set_style_text_color(bt_status_label, lv_color_red(), 0);
    }
    
    // Update notifications
    if (m_watch_state.has_notification) {
        lv_label_set_text(notification_label, m_watch_state.notification_text);
    } else {
        lv_label_set_text(notification_label, "");
    }
}

/**@brief Timer handler for display updates.
 */
static void display_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    update_display();
    lv_timer_handler();
}

/**@brief Timer handler for step counting.
 */
static void step_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    m_watch_state.step_count = step_counter_get_count();
}

/**@brief Timer handler for time updates.
 */
static void time_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    time_manager_tick();
}

/**@brief Create application timers.
 */
static void create_timers(void)
{
    ret_code_t err_code;

    // Display update timer (100ms)
    err_code = app_timer_create(&m_display_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                display_timer_handler);
    APP_ERROR_CHECK(err_code);

    // Step counter timer (1s)
    err_code = app_timer_create(&m_step_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                step_timer_handler);
    APP_ERROR_CHECK(err_code);

    // Time update timer (1s)
    err_code = app_timer_create(&m_time_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                time_timer_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Start application timers.
 */
static void start_timers(void)
{
    ret_code_t err_code;

    err_code = app_timer_start(m_display_timer_id, APP_TIMER_TICKS(100), NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_step_timer_id, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_time_timer_id, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    timers_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    advertising_init();

    // Initialize hardware components
    st7789_init();
    step_counter_init();
    time_manager_init();
    notification_service_init_simple();

    // Initialize LVGL and create UI
    lvgl_init();
    create_watch_ui();
    
    // Create and start timers
    create_timers();
    start_timers();

    // Start advertising.
    advertising_start();

    NRF_LOG_INFO("nRF40 Smartwatch started.");

    // Enter main loop.
    for (;;)
    {
        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();
        }
    }
}