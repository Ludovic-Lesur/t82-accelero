/*
 * fxls89xxxx_hw.c
 *
 *  Created on: 27 apr. 2027
 *      Author: Ludo
 */

#include "fxls89xxxx_hw.h"

#include "error.h"
#include "error_base.h"
#include "i2c.h"
#include "mcu_mapping.h"
#include "types.h"

/*** SENSORS HW functions ***/

/*******************************************************************/
FXLS89XXXX_status_t FXLS89XXXX_HW_init(void) {
    // Local variables.
    FXLS89XXXX_status_t status = FXLS89XXXX_SUCCESS;
    I2C_status_t i2c_status = I2C_SUCCESS;
    // Init I2C.
    i2c_status = I2C_init(I2C_INSTANCE_SENSORS, &I2C_GPIO_SENSORS);
    I2C_exit_error(FXLS89XXXX_ERROR_BASE_I2C);
errors:
    return status;
}

/*******************************************************************/
FXLS89XXXX_status_t FXLS89XXXX_HW_de_init(void) {
    // Local variables.
    FXLS89XXXX_status_t status = FXLS89XXXX_SUCCESS;
    I2C_status_t i2c_status = I2C_SUCCESS;
    // Init I2C.
    i2c_status = I2C_de_init(I2C_INSTANCE_SENSORS, &I2C_GPIO_SENSORS);
    I2C_stack_error(ERROR_BASE_FXLS8974CF + FXLS89XXXX_ERROR_BASE_I2C);
    return status;
}

/*******************************************************************/
FXLS89XXXX_status_t FXLS89XXXX_HW_i2c_write(uint8_t i2c_address, uint8_t* data, uint8_t data_size_bytes, uint8_t stop_flag) {
    // Local variables.
    FXLS89XXXX_status_t status = FXLS89XXXX_SUCCESS;
    I2C_status_t i2c_status = I2C_SUCCESS;
    // I2C transfer.
    i2c_status = I2C_write(I2C_INSTANCE_SENSORS, i2c_address, data, data_size_bytes, stop_flag);
    I2C_exit_error(FXLS89XXXX_ERROR_BASE_I2C);
errors:
    return status;
}

/*******************************************************************/
FXLS89XXXX_status_t FXLS89XXXX_HW_i2c_read(uint8_t i2c_address, uint8_t* data, uint8_t data_size_bytes) {
    // Local variables.
    FXLS89XXXX_status_t status = FXLS89XXXX_SUCCESS;
    I2C_status_t i2c_status = I2C_SUCCESS;
    // I2C transfer.
    i2c_status = I2C_read(I2C_INSTANCE_SENSORS, i2c_address, data, data_size_bytes);
    I2C_exit_error(FXLS89XXXX_ERROR_BASE_I2C);
errors:
    return status;
}
