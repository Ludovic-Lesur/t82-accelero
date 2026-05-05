/*
 * mcu_mapping.c
 *
 *  Created on: 27 apr. 2026
 *      Author: Ludo
 */

#include "mcu_mapping.h"

#include "gpio.h"
#include "gpio_registers.h"
#include "i2c.h"

/*** GPIO MAPPING local global variables ***/

// I2C sensors.
static const GPIO_pin_t GPIO_I2C1_SCL = { GPIOB, 1, 6, 1 };
static const GPIO_pin_t GPIO_I2C1_SDA = { GPIOB, 1, 7, 1 };

/*** GPIO MAPPING global variables ***/

// Accelerometer.
const GPIO_pin_t GPIO_ACCELERO_IRQ = { GPIOA, 0, 0, 0 };
// T82 interface.
const GPIO_pin_t GPIO_T82_BUTTON = { GPIOA, 0, 5, 0 };
const GPIO_pin_t GPIO_T82_ALARM = { GPIOA, 0, 6, 0 };
const GPIO_pin_t GPIO_T82_LED = { GPIOA, 0, 7, 1 };
// Sensors.
const I2C_gpio_t I2C_GPIO_SENSORS = { &GPIO_I2C1_SCL, &GPIO_I2C1_SDA };
// Test point.
const GPIO_pin_t GPIO_TP1 = { GPIOA, 0, 1, 0 };
