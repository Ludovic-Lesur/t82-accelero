/*
 * mcu_mapping.h
 *
 *  Created on: 27 apr. 2026
 *      Author: Ludo
 */

#ifndef __MCU_MAPPING_H__
#define __MCU_MAPPING_H__

#include "gpio.h"
#include "i2c.h"

/*** MCU MAPPING macros ***/

#define I2C_INSTANCE_SENSORS    I2C_INSTANCE_I2C1

/*** MCU MAPPING global variables ***/

// Accelerometer.
extern const GPIO_pin_t GPIO_ACCELERO_IRQ;
// T82 interface.
extern const GPIO_pin_t GPIO_T82_BUTTON;
extern const GPIO_pin_t GPIO_T82_ALARM;
extern const GPIO_pin_t GPIO_T82_LED;
// Sensors.
extern const I2C_gpio_t I2C_GPIO_SENSORS;
// Test point.
extern const GPIO_pin_t GPIO_TP1;

#endif /* __MCU_MAPPING_H__ */
