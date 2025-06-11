#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "nrf_log.h"

// Time structure
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday; // 0 = Sunday, 6 = Saturday
} watch_time_t;

static watch_time_t m_current_time = {0};
static bool m_time_initialized = false;
static uint32_t m_tick_counter = 0;

// Month names
static const char* month_names[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// Weekday names
static const char* weekday_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

// Days in each month (non-leap year)
static const uint8_t days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/**@brief Check if year is leap year
 */
static bool time_is_leap_year(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/**@brief Get days in month for given year
 */
static uint8_t time_get_days_in_month(uint8_t month, uint16_t year)
{
    if (month == 2 && time_is_leap_year(year)) {
        return 29;
    }
    return days_in_month[month - 1];
}

/**@brief Calculate weekday from date (Zeller's congruence)
 */
static uint8_t time_calculate_weekday(uint8_t day, uint8_t month, uint16_t year)
{
    if (month < 3) {
        month += 12;
        year--;
    }
    
    uint8_t k = year % 100;
    uint8_t j = year / 100;
    
    uint8_t h = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) - 2 * j) % 7;
    
    // Convert to Sunday = 0 format
    return (h + 5) % 7;
}

/**@brief Initialize time manager
 */
void time_manager_init(void)
{
    // Initialize with default time (12:00:00, Jan 1, 2025, Monday)
    m_current_time.hours = 12;
    m_current_time.minutes = 0;
    m_current_time.seconds = 0;
    m_current_time.day = 1;
    m_current_time.month = 1;
    m_current_time.year = 2025;
    m_current_time.weekday = time_calculate_weekday(m_current_time.day, 
                                                   m_current_time.month, 
                                                   m_current_time.year);
    
    m_time_initialized = true;
    m_tick_counter = 0;
    
    NRF_LOG_INFO("Time manager initialized: %02d:%02d:%02d %02d/%02d/%04d", 
                 m_current_time.hours, m_current_time.minutes, m_current_time.seconds,
                 m_current_time.day, m_current_time.month, m_current_time.year);
}

/**@brief Tick function called every second
 */
void time_manager_tick(void)
{
    if (!m_time_initialized) {
        return;
    }
    
    m_tick_counter++;
    m_current_time.seconds++;
    
    // Handle seconds overflow
    if (m_current_time.seconds >= 60) {
        m_current_time.seconds = 0;
        m_current_time.minutes++;
        
        // Handle minutes overflow
        if (m_current_time.minutes >= 60) {
            m_current_time.minutes = 0;
            m_current_time.hours++;
            
            // Handle hours overflow
            if (m_current_time.hours >= 24) {
                m_current_time.hours = 0;
                m_current_time.day++;
                m_current_time.weekday = (m_current_time.weekday + 1) % 7;
                
                // Handle day overflow
                uint8_t max_days = time_get_days_in_month(m_current_time.month, m_current_time.year);
                if (m_current_time.day > max_days) {
                    m_current_time.day = 1;
                    m_current_time.month++;
                    
                    // Handle month overflow
                    if (m_current_time.month > 12) {
                        m_current_time.month = 1;
                        m_current_time.year++;
                    }
                }
            }
        }
    }
}

/**@brief Set time
 */
void time_manager_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (hours < 24 && minutes < 60 && seconds < 60) {
        m_current_time.hours = hours;
        m_current_time.minutes = minutes;
        m_current_time.seconds = seconds;
        
        NRF_LOG_INFO("Time set to: %02d:%02d:%02d", hours, minutes, seconds);
    }
}

/**@brief Set date
 */
void time_manager_set_date(uint8_t day, uint8_t month, uint16_t year)
{
    if (day >= 1 && day <= 31 && month >= 1 && month <= 12 && year >= 2000) {
        uint8_t max_days = time_get_days_in_month(month, year);
        if (day <= max_days) {
            m_current_time.day = day;
            m_current_time.month = month;
            m_current_time.year = year;
            m_current_time.weekday = time_calculate_weekday(day, month, year);
            
            NRF_LOG_INFO("Date set to: %02d/%02d/%04d", day, month, year);
        }
    }
}

/**@brief Get current time
 */
watch_time_t time_manager_get_time(void)
{
    return m_current_time;
}

/**@brief Get time as formatted string (HH:MM)
 */
void time_manager_get_time_string(char* buffer, size_t buffer_size)
{
    if (buffer && buffer_size >= 6) {
        snprintf(buffer, buffer_size, "%02d:%02d", 
                m_current_time.hours, m_current_time.minutes);
    }
}

/**@brief Get time with seconds as formatted string (HH:MM:SS)
 */
void time_manager_get_time_string_with_seconds(char* buffer, size_t buffer_size)
{
    if (buffer && buffer_size >= 9) {
        snprintf(buffer, buffer_size, "%02d:%02d:%02d", 
                m_current_time.hours, m_current_time.minutes, m_current_time.seconds);
    }
}

/**@brief Get date as formatted string (DD/MM/YYYY)
 */
void time_manager_get_date_string(char* buffer, size_t buffer_size)
{
    if (buffer && buffer_size >= 11) {
        snprintf(buffer, buffer_size, "%02d/%02d/%04d", 
                m_current_time.day, m_current_time.month, m_current_time.year);
    }
}

/**@brief Get date as formatted string with month name (DD MMM YYYY)
 */
void time_manager_get_date_string_with_month_name(char* buffer, size_t buffer_size)
{
    if (buffer && buffer_size >= 12 && m_current_time.month >= 1 && m_current_time.month <= 12) {
        snprintf(buffer, buffer_size, "%02d %s %04d", 
                m_current_time.day, 
                month_names[m_current_time.month - 1], 
                m_current_time.year);
    }
}

/**@brief Get weekday name
 */
const char* time_manager_get_weekday_name(void)
{
    if (m_current_time.weekday < 7) {
        return weekday_names[m_current_time.weekday];
    }
    return "Unknown";
}

/**@brief Get uptime in seconds
 */
uint32_t time_manager_get_uptime_seconds(void)
{
    return m_tick_counter;
}

/**@brief Check if time is initialized
 */
bool time_manager_is_initialized(void)
{
    return m_time_initialized;
}

/**@brief Convert to Unix timestamp (approximate)
 */
uint32_t time_manager_get_unix_timestamp(void)
{
    // Simple conversion - not accounting for all leap years/seconds
    // This is an approximation for basic functionality
    uint32_t days_since_epoch = 0;
    
    // Add days for complete years since 1970
    for (uint16_t year = 1970; year < m_current_time.year; year++) {
        days_since_epoch += time_is_leap_year(year) ? 366 : 365;
    }
    
    // Add days for complete months in current year
    for (uint8_t month = 1; month < m_current_time.month; month++) {
        days_since_epoch += time_get_days_in_month(month, m_current_time.year);
    }
    
    // Add days in current month
    days_since_epoch += m_current_time.day - 1;
    
    // Convert to seconds and add time of day
    uint32_t timestamp = days_since_epoch * 86400; // 24*60*60
    timestamp += m_current_time.hours * 3600;
    timestamp += m_current_time.minutes * 60;
    timestamp += m_current_time.seconds;
    
    return timestamp;
}

#endif // TIME_MANAGER_H
