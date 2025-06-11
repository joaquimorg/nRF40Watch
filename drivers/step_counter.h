#ifndef STEP_COUNTER_H
#define STEP_COUNTER_H

#include <stdint.h>
#include <stdbool.h>
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "app_error.h"

// BMA400 I2C address and registers (based on your schematic)
#define BMA400_I2C_ADDR         0x14
#define BMA400_CHIP_ID_REG      0x00
#define BMA400_CHIP_ID_VALUE    0x90
#define BMA400_ACC_CONFIG0_REG  0x19
#define BMA400_ACC_CONFIG1_REG  0x1A
#define BMA400_ACC_CONFIG2_REG  0x1B
#define BMA400_INT_CONFIG0_REG  0x1F
#define BMA400_INT_CONFIG1_REG  0x20
#define BMA400_INT1_MAP_REG     0x21
#define BMA400_STEP_CNT_0_REG   0x15
#define BMA400_STEP_CNT_1_REG   0x16
#define BMA400_STEP_CNT_2_REG   0x17
#define BMA400_STEP_CONF_0_REG  0x59
#define BMA400_STEP_CONF_1_REG  0x5A

// I2C instance
#define TWI_INSTANCE_ID     0
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

// I2C pins based on your schematic
#define TWI_SCL_PIN         22
#define TWI_SDA_PIN         23

// Step counter state
static uint32_t m_step_count = 0;
static bool m_step_counter_initialized = false;

/**@brief TWI event handler
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    // TWI transaction complete
}

/**@brief Write register to BMA400
 */
static ret_code_t bma400_write_reg(uint8_t reg_addr, uint8_t value)
{
    uint8_t data[2] = {reg_addr, value};
    return nrf_drv_twi_tx(&m_twi, BMA400_I2C_ADDR, data, sizeof(data), false);
}

/**@brief Read register from BMA400
 */
static ret_code_t bma400_read_reg(uint8_t reg_addr, uint8_t* p_data, uint8_t length)
{
    ret_code_t err_code;
    
    err_code = nrf_drv_twi_tx(&m_twi, BMA400_I2C_ADDR, &reg_addr, 1, true);
    if (err_code != NRF_SUCCESS) return err_code;
    
    return nrf_drv_twi_rx(&m_twi, BMA400_I2C_ADDR, p_data, length);
}

/**@brief Initialize step counter
 */
void step_counter_init(void)
{
    ret_code_t err_code;
    uint8_t chip_id;
    
    // Initialize TWI
    nrf_drv_twi_config_t const config = {
       .scl                = TWI_SCL_PIN,
       .sda                = TWI_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };
    
    err_code = nrf_drv_twi_init(&m_twi, &config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&m_twi);
    
    // Verify chip ID
    err_code = bma400_read_reg(BMA400_CHIP_ID_REG, &chip_id, 1);
    if (err_code != NRF_SUCCESS || chip_id != BMA400_CHIP_ID_VALUE) {
        NRF_LOG_ERROR("BMA400 not found or wrong chip ID: 0x%02X", chip_id);
        return;
    }
    
    NRF_LOG_INFO("BMA400 found, chip ID: 0x%02X", chip_id);
    
    // Configure accelerometer
    // Set ODR to 100Hz, range to ±2g
    err_code = bma400_write_reg(BMA400_ACC_CONFIG0_REG, 0x08); // ODR 100Hz
    APP_ERROR_CHECK(err_code);
    
    err_code = bma400_write_reg(BMA400_ACC_CONFIG1_REG, 0x00); // Range ±2g
    APP_ERROR_CHECK(err_code);
    
    // Enable step counter
    err_code = bma400_write_reg(BMA400_STEP_CONF_0_REG, 0x01); // Enable step counter
    APP_ERROR_CHECK(err_code);
    
    // Configure step detection parameters
    err_code = bma400_write_reg(BMA400_STEP_CONF_1_REG, 0x15); // Step threshold and sensitivity
    APP_ERROR_CHECK(err_code);
    
    nrf_delay_ms(50); // Allow sensor to stabilize
    
    m_step_counter_initialized = true;
    NRF_LOG_INFO("Step counter initialized successfully");
}

/**@brief Get current step count
 */
uint32_t step_counter_get_count(void)
{
    if (!m_step_counter_initialized) {
        return 0;
    }
    
    ret_code_t err_code;
    uint8_t step_data[3];
    
    // Read step count registers (24-bit value)
    err_code = bma400_read_reg(BMA400_STEP_CNT_0_REG, step_data, 3);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to read step count");
        return m_step_count;
    }
    
    // Combine 3 bytes into 32-bit step count
    m_step_count = (uint32_t)step_data[0] | 
                   ((uint32_t)step_data[1] << 8) | 
                   ((uint32_t)step_data[2] << 16);
    
    return m_step_count;
}

/**@brief Reset step counter
 */
void step_counter_reset(void)
{
    if (!m_step_counter_initialized) {
        return;
    }
    
    // Reset step counter by disabling and re-enabling
    bma400_write_reg(BMA400_STEP_CONF_0_REG, 0x00); // Disable
    nrf_delay_ms(10);
    bma400_write_reg(BMA400_STEP_CONF_0_REG, 0x01); // Re-enable
    
    m_step_count = 0;
    NRF_LOG_INFO("Step counter reset");
}

/**@brief Check if step counter is working
 */
bool step_counter_is_initialized(void)
{
    return m_step_counter_initialized;
}

#endif // STEP_COUNTER_H