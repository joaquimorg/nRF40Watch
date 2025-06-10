// --- config/sdk_config.h ---

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

//==========================================================
// <h> Application
//==========================================================
#define APP_TIMER_ENABLED 1
#define APP_TIMER_CONFIG_RTC_FREQUENCY 0
#define APP_TIMER_CONFIG_IRQ_PRIORITY 6
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 4
#define APP_TIMER_CONFIG_USE_SCHEDULER 0
#define APP_TIMER_KEEPS_RTC_ACTIVE 0
#define APP_TIMER_CONFIG_SWI_NUMBER 0
// </h>

//==========================================================
// <h> SoftDevice
//==========================================================
#define NRF_SDH_ENABLED 1
#define NRF_SDH_CLOCK_LF_SRC 1
#define NRF_SDH_CLOCK_LF_RC_CTIV 16
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV 2
#define NRF_SDH_CLOCK_LF_ACCURACY 1
#define NRF_SDH_BLE_ENABLED 1
#define NRF_SDH_BLE_VS_UUID_COUNT 1
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1
#define NRF_SDH_BLE_GAP_DATA_LENGTH 27
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 247
#define NRF_SDH_BLE_OBSERVER_PRIO_LEVELS 4
#define NRF_SDH_STACK_OBSERVER_PRIO_LEVELS 3
#define NRF_SDH_STATE_OBSERVER_PRIO_LEVELS 3
#define NRF_SDH_REQ_OBSERVER_PRIO_LEVELS 3
#define NRF_SDH_BLE_STACK_OBSERVER_PRIO 0
#define NRF_SDH_BLE_STACK_OBSERVER_PRIO_LEVELS 3
// </h>

//==========================================================
// <h> BLE GAP Parameters
//==========================================================
#define BLE_GAP_ENABLED 1
#define BLE_ADVERTISING_ENABLED 1
// </h>

//==========================================================
// <h> GPIOTE
//==========================================================
#define GPIOTE_ENABLED 1
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 1
// </h>

//==========================================================
// <h> TWI (I2C) driver
//==========================================================
#define NRFX_TWIM_ENABLED 1
#define NRFX_TWIM0_ENABLED 1
#define NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_TWIM_DEFAULT_CONFIG_FREQUENCY 267386880
#define NRFX_TWIM_DEFAULT_CONFIG_HOLD_BUS_UNINIT 0
// </h>

//==========================================================
// <h> SPI driver
//==========================================================
#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM0_ENABLED 1
#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_SPIM_MISO_PULL_CFG 1
#define NRFX_SPIM_DEFAULT_CONFIG_FREQUENCY 267386880
#define NRFX_SPIM_DEFAULT_CONFIG_MODE 0
#define NRFX_SPIM_DEFAULT_CONFIG_BIT_ORDER 0
#define NRFX_SPIM_CONFIG_LOG_ENABLED 0
// </h>

//==========================================================
// <h> Logging (optional)
//==========================================================
#define NRF_LOG_ENABLED 0
// </h>

// <e> NRFX_POWER_ENABLED - nrfx_power - POWER peripheral driver
//==========================================================
#ifndef NRFX_POWER_ENABLED
#define NRFX_POWER_ENABLED 1
#endif
// <o> NRFX_POWER_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef NRFX_POWER_CONFIG_IRQ_PRIORITY
#define NRFX_POWER_CONFIG_IRQ_PRIORITY 6
#endif

// <q> NRFX_POWER_CONFIG_DEFAULT_DCDCEN  - The default configuration of main DCDC regulator
 

// <i> This settings means only that components for DCDC regulator are installed and it can be enabled.

#ifndef NRFX_POWER_CONFIG_DEFAULT_DCDCEN
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN 0
#endif

// <q> NRFX_POWER_CONFIG_DEFAULT_DCDCENHV  - The default configuration of High Voltage DCDC regulator
 

// <i> This settings means only that components for DCDC regulator are installed and it can be enabled.

#ifndef NRFX_POWER_CONFIG_DEFAULT_DCDCENHV
#define NRFX_POWER_CONFIG_DEFAULT_DCDCENHV 0
#endif

// </e>

// <e> NRFX_CLOCK_ENABLED - nrfx_clock - CLOCK peripheral driver
//==========================================================
#ifndef NRFX_CLOCK_ENABLED
#define NRFX_CLOCK_ENABLED 0
#endif
// <o> NRFX_CLOCK_CONFIG_LF_SRC  - LF Clock Source
 
// <0=> RC 
// <1=> XTAL 
// <2=> Synth 
// <131073=> External Low Swing 
// <196609=> External Full Swing 

#ifndef NRFX_CLOCK_CONFIG_LF_SRC
#define NRFX_CLOCK_CONFIG_LF_SRC 1
#endif

// <o> NRFX_CLOCK_CONFIG_IRQ_PRIORITY  - Interrupt priority
 
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef NRFX_CLOCK_CONFIG_IRQ_PRIORITY
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_CLOCK_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_CLOCK_CONFIG_LOG_ENABLED
#define NRFX_CLOCK_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_CLOCK_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRFX_CLOCK_CONFIG_LOG_LEVEL
#define NRFX_CLOCK_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_CLOCK_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRFX_CLOCK_CONFIG_INFO_COLOR
#define NRFX_CLOCK_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_CLOCK_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRFX_CLOCK_CONFIG_DEBUG_COLOR
#define NRFX_CLOCK_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

#endif // SDK_CONFIG_H

