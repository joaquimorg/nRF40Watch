#ifndef NOTIFICATION_SERVICE_H
#define NOTIFICATION_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_log.h"

// Custom UUID for notification service
#define NOTIFICATION_SERVICE_UUID_BASE { 0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00 }
#define NOTIFICATION_SERVICE_UUID      0x1523
#define NOTIFICATION_CHAR_UUID         0x1524

// Maximum notification text length
#define MAX_NOTIFICATION_LENGTH        256

#define BLE_NOTIFICATION_SERVICE_BLE_OBSERVER_PRIO 2

// Notification types
typedef enum {
    NOTIFICATION_TYPE_CALL      = 0x01,
    NOTIFICATION_TYPE_SMS       = 0x02,
    NOTIFICATION_TYPE_EMAIL     = 0x03,
    NOTIFICATION_TYPE_APP       = 0x04,
    NOTIFICATION_TYPE_SOCIAL    = 0x05,
    NOTIFICATION_TYPE_CALENDAR  = 0x06,
    NOTIFICATION_TYPE_OTHER     = 0xFF
} notification_type_t;

// Notification structure
typedef struct {
    notification_type_t type;
    char title[64];
    char content[192];
    uint32_t timestamp;
    bool is_active;
} notification_t;

// Forward declaration
typedef struct notification_service_s notification_service_t;

// Event handler type
typedef void (*notification_evt_handler_t)(notification_service_t * p_service, notification_t * p_notification);

// Service structure
struct notification_service_s {
    uint8_t                      uuid_type;
    uint16_t                     service_handle;
    ble_gatts_char_handles_t     notification_char_handles;
    uint16_t                     conn_handle;
    notification_evt_handler_t   evt_handler;
};

// Service initialization structure
typedef struct {
    notification_evt_handler_t evt_handler;
} notification_service_init_t;

// Global service instance
static notification_service_t m_notification_service;
static notification_t m_current_notification = {0};
static bool m_service_initialized = false;

/**@brief Function for handling BLE events related to the notification service.
 */
static void on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    notification_service_t * p_service = (notification_service_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            p_service->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GATTS_EVT_WRITE:
        {
            ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
            
            if (p_evt_write->handle == p_service->notification_char_handles.value_handle &&
                p_evt_write->len > 0)
            {
                // Parse received notification data
                uint8_t* data = p_evt_write->data;
                uint16_t len = p_evt_write->len;
                
                if (len >= 1) {
                    // First byte is notification type
                    m_current_notification.type = (notification_type_t)data[0];
                    m_current_notification.is_active = true;
                    m_current_notification.timestamp = time_manager_get_unix_timestamp();
                    
                    // Parse title and content (simple format: type|title|content)
                    uint16_t offset = 1;
                    uint16_t title_len = 0;
                    uint16_t content_len = 0;
                    
                    // Find title length (until first null or separator)
                    while (offset + title_len < len && 
                           data[offset + title_len] != 0 && 
                           data[offset + title_len] != '|' &&
                           title_len < sizeof(m_current_notification.title) - 1) {
                        title_len++;
                    }
                    
                    // Copy title
                    if (title_len > 0) {
                        memcpy(m_current_notification.title, &data[offset], title_len);
                    }
                    m_current_notification.title[title_len] = 0;
                    offset += title_len + 1; // Skip separator
                    
                    // Copy remaining as content
                    if (offset < len) {
                        content_len = len - offset;
                        if (content_len > sizeof(m_current_notification.content) - 1) {
                            content_len = sizeof(m_current_notification.content) - 1;
                        }
                        memcpy(m_current_notification.content, &data[offset], content_len);
                    }
                    m_current_notification.content[content_len] = 0;
                    
                    NRF_LOG_INFO("Notification received - Type: %d, Title: %s, Content: %s", 
                                m_current_notification.type,
                                m_current_notification.title,
                                m_current_notification.content);
                    
                    // Call event handler if set
                    if (p_service->evt_handler) {
                        p_service->evt_handler(p_service, &m_current_notification);
                    }
                }
            }
            break;
        }

        default:
            break;
    }
}

/**@brief Function for adding notification characteristic.
 */
static uint32_t notification_char_add(notification_service_t * p_service)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write   = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.char_props.notify  = 1;
    char_md.p_char_user_desc   = NULL;
    char_md.p_char_pf          = NULL;
    char_md.p_user_desc_md     = NULL;
    char_md.p_cccd_md          = &cccd_md;
    char_md.p_sccd_md          = NULL;

    ble_uuid.type = p_service->uuid_type;
    ble_uuid.uuid = NOTIFICATION_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_NOTIFICATION_LENGTH;

    return sd_ble_gatts_characteristic_add(p_service->service_handle,
                                          &char_md,
                                          &attr_char_value,
                                          &p_service->notification_char_handles);
}

/**@brief Initialize notification service
 */
uint32_t notification_service_init(notification_service_init_t * p_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t base_uuid = NOTIFICATION_SERVICE_UUID_BASE;

    if (p_init == NULL) {
        return NRF_ERROR_NULL;
    }

    // Initialize service structure
    m_notification_service.conn_handle = BLE_CONN_HANDLE_INVALID;
    m_notification_service.evt_handler = p_init->evt_handler;

    // Add custom base UUID
    err_code = sd_ble_uuid_vs_add(&base_uuid, &m_notification_service.uuid_type);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    ble_uuid.type = m_notification_service.uuid_type;
    ble_uuid.uuid = NOTIFICATION_SERVICE_UUID;

    // Add service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                       &ble_uuid,
                                       &m_notification_service.service_handle);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // Add notification characteristic
    err_code = notification_char_add(&m_notification_service);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // Register BLE event handler
    NRF_SDH_BLE_OBSERVER(m_notification_service_observer, 
                        BLE_NOTIFICATION_SERVICE_BLE_OBSERVER_PRIO, 
                        on_ble_evt, 
                        &m_notification_service);

    m_service_initialized = true;
    NRF_LOG_INFO("Notification service initialized");

    return NRF_SUCCESS;
}

/**@brief Simple init function without parameters
 */
void notification_service_init(void)
{
    notification_service_init_t init = {0};
    uint32_t err_code = notification_service_init(&init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Get current notification
 */
notification_t* notification_service_get_current(void)
{
    if (m_current_notification.is_active) {
        return &m_current_notification;
    }
    return NULL;
}

/**@brief Clear current notification
 */
void notification_service_clear_current(void)
{
    memset(&m_current_notification, 0, sizeof(m_current_notification));
    m_current_notification.is_active = false;
}

/**@brief Check if service is initialized
 */
bool notification_service_is_initialized(void)
{
    return m_service_initialized;
}

/**@brief Get notification type as string
 */
const char* notification_service_get_type_string(notification_type_t type)
{
    switch (type) {
        case NOTIFICATION_TYPE_CALL:     return "Call";
        case NOTIFICATION_TYPE_SMS:      return "SMS";
        case NOTIFICATION_TYPE_EMAIL:    return "Email";
        case NOTIFICATION_TYPE_APP:      return "App";
        case NOTIFICATION_TYPE_SOCIAL:   return "Social";
        case NOTIFICATION_TYPE_CALENDAR: return "Calendar";
        default:                         return "Other";
    }
}

/**@brief Format notification for display
 */
void notification_service_format_for_display(char* buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0) return;
    
    if (m_current_notification.is_active) {
        if (strlen(m_current_notification.title) > 0) {
            snprintf(buffer, buffer_size, "%s: %s", 
                    m_current_notification.title,
                    m_current_notification.content);
        } else {
            snprintf(buffer, buffer_size, "%s", m_current_notification.content);
        }
    } else {
        buffer[0] = 0;
    }
}

#endif // NOTIFICATION_SERVICE_H