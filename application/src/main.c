/*
 * main.c
 *
 *  Created on: 27 apr. 2026
 *      Author: Ludo
 */

// Peripherals.
#include "exti.h"
#include "gpio.h"
#include "i2c_address.h"
#include "iwdg.h"
#include "lptim.h"
#include "maths.h"
#include "mcu_mapping.h"
#include "nvic.h"
#include "nvic_priority.h"
#include "pwr.h"
#include "rcc.h"
#include "rtc.h"
// Components.
#include "fxls89xxxx.h"
// Utils.
#include "error.h"
#include "types.h"
// Applicative.
#include "error_base.h"
#include "t82_accelero_flags.h"

/*** MAIN macros ***/

#define T82_ACCELERO_ARMING_PRESS_DURATION_SECONDS  5

#define T82_ACCELERO_ARMING_TIME_SECONDS            30
#define T82_ACCELERO_ARMING_LED_PULSE_MS            200
#define T82_ACCELERO_ARMING_LED_PERIOD_MS           1000
#define T82_ACCELERO_ARMING_LED_FREQUENCY_MHZ       (1000000 / T82_ACCELERO_ARMING_LED_PERIOD_MS)
#define T82_ACCELERO_ARMING_LED_DUTY_CYCLE_PERCENT  ((MATH_PERCENT_MAX * T82_ACCELERO_ARMING_LED_PULSE_MS) / T82_ACCELERO_ARMING_LED_PERIOD_MS)

#define T82_ACCELERO_ACTIVE_LED_PULSE_MS            200
#define T82_ACCELERO_ACTIVE_LED_PERIOD_MS           3000
#define T82_ACCELERO_ACTIVE_LED_FREQUENCY_MHZ       (1000000 / T82_ACCELERO_ACTIVE_LED_PERIOD_MS)
#define T82_ACCELERO_ACTIVE_LED_DUTY_CYCLE_PERCENT  ((MATH_PERCENT_MAX * T82_ACCELERO_ACTIVE_LED_PULSE_MS) / T82_ACCELERO_ACTIVE_LED_PERIOD_MS)

#define T82_ACCELERO_WAKE_UP_PULSE_MS               1000

#define T82_ACCELERO_ALARM_PULSE_MS                 5000

#define T82_ACCELERO_CONFIGURATION_SIZE_ACTIVE      12
#define T82_ACCELERO_CONFIGURATION_SIZE_SLEEP       1

/*** MAIN structures ***/

/*******************************************************************/
typedef enum {
    T82_ACCELERO_STATE_INIT = 0,
    T82_ACCELERO_STATE_READY,
    T82_ACCELERO_STATE_ARMING_CONFIRMATION,
    T82_ACCELERO_STATE_ARMING,
    T82_ACCELERO_STATE_ACTIVE,
    T82_ACCELERO_STATE_ALARM,
    T82_ACCELERO_STATE_LAST
} T82_ACCELERO_state_t;

/*******************************************************************/
typedef union {
    uint8_t all;
    struct {
        unsigned button_pressed: 1;
        unsigned motion_detected :1;
    };
} T82_ACCELERO_flags_t;

/*******************************************************************/
typedef struct {
    T82_ACCELERO_state_t state;
    volatile T82_ACCELERO_flags_t flags;
    uint32_t button_press_start_time;
    uint32_t arming_start_time;
} T82_ACCELERO_context_t;

/*** MAIN local global variables ***/

static const FXLS89XXXX_register_setting_t ACCELEROMETER_CONFIGURATION_ACTIVE[T82_ACCELERO_CONFIGURATION_SIZE_ACTIVE] = {
    { FXLS89XXXX_REGISTER_SENS_CONFIG1, 0x00 }, // // Full scale = +/-2g.
    { FXLS89XXXX_REGISTER_SENS_CONFIG3, 0x99 }, // ODR = 6.25Hz.
    { FXLS89XXXX_REGISTER_SDCD_UTHS_MSB, 0x00 }, // High threshold delta = +0.1g.
    { FXLS89XXXX_REGISTER_SDCD_UTHS_LSB, 0x66 },
    { FXLS89XXXX_REGISTER_SDCD_LTHS_MSB, 0x0F }, // Low threshold delta = -0.1g.
    { FXLS89XXXX_REGISTER_SDCD_LTHS_LSB, 0x9A },
    { FXLS89XXXX_REGISTER_INT_EN, 0x20 }, // Enable outside threshold interrupt.
    { FXLS89XXXX_REGISTER_SENS_CONFIG4, 0x01 }, // INT polarity is active high.
    { FXLS89XXXX_REGISTER_INT_PIN_SEL, 0x00 }, // Use INT1 pin.
    { FXLS89XXXX_REGISTER_SDCD_CONFIG1, 0x38 }, // Disable event latch and enable interrupts on all axis.
    { FXLS89XXXX_REGISTER_SDCD_CONFIG2, 0xD8 }, // Enable SDCD, enable relative mode and disable debouncing.
    { FXLS89XXXX_REGISTER_SENS_CONFIG1, 0x01 } // ACTIVE='1'.
};

static const FXLS89XXXX_register_setting_t ACCELEROMETER_CONFIGURATION_SLEEP[T82_ACCELERO_CONFIGURATION_SIZE_SLEEP] = {
    { FXLS89XXXX_REGISTER_SENS_CONFIG1, 0x00 }, // ACTIVE='0'.
};

static T82_ACCELERO_context_t t82_accelero_ctx;

/*** MAIN functions ***/

/*******************************************************************/
static void _T82_ACCELERO_button_irq_callback(void) {
    // Set button flag.
    t82_accelero_ctx.flags.button_pressed = 1;
}

/*******************************************************************/
static void _T82_ACCELERO_motion_irq_callback(void) {
    // Set motion flag.
    t82_accelero_ctx.flags.motion_detected = 1;
}

/*******************************************************************/
static void _T82_ACCELERO_disable_motion_irq(void) {
    // Disable interrupt.
    EXTI_disable_gpio_interrupt(&GPIO_ACCELERO_IRQ);
    // Release GPIO.
    EXTI_release_gpio(&GPIO_ACCELERO_IRQ, GPIO_MODE_INPUT);
    // Disable accelerometer.
    FXLS89XXXX_write_configuration(I2C_ADDRESS_FXLS8974CF, &(ACCELEROMETER_CONFIGURATION_SLEEP[0]), T82_ACCELERO_CONFIGURATION_SIZE_SLEEP);
}

/*******************************************************************/
static void _T82_ACCELERO_enable_motion_irq(void) {
    // Enable accelerometer.
    FXLS89XXXX_write_configuration(I2C_ADDRESS_FXLS8974CF, &(ACCELEROMETER_CONFIGURATION_ACTIVE[0]), T82_ACCELERO_CONFIGURATION_SIZE_ACTIVE);
    // Configure GPIO.
    EXTI_configure_gpio(&GPIO_ACCELERO_IRQ, GPIO_PULL_NONE, EXTI_TRIGGER_RISING_EDGE, &_T82_ACCELERO_motion_irq_callback, NVIC_PRIORITY_ACCELEROMETER);
    // Enable interrupt.
    EXTI_clear_gpio_flag(&GPIO_ACCELERO_IRQ);
    EXTI_enable_gpio_interrupt(&GPIO_ACCELERO_IRQ);
}

/*******************************************************************/
static void _T82_ACCELERO_init_context(void) {
    // Init state machine.
    t82_accelero_ctx.state = T82_ACCELERO_STATE_INIT;
    t82_accelero_ctx.flags.all = 0;
    t82_accelero_ctx.button_press_start_time = 0;
    t82_accelero_ctx.arming_start_time = 0;
}

/*******************************************************************/
static void _T82_ACCELERO_init_hw(void) {
    // Local variables.
    RCC_status_t rcc_status = RCC_SUCCESS;
    RTC_status_t rtc_status = RTC_SUCCESS;
    LPTIM_status_t lptim_status = LPTIM_SUCCESS;
#ifndef T82_ACCELERO_MODE_DEBUG
    IWDG_status_t iwdg_status = IWDG_SUCCESS;
#endif
    // Init error stack
    ERROR_stack_init();
    // Init memory.
    NVIC_init();
    // Init power module and clock tree.
    PWR_init();
    rcc_status = RCC_init(NVIC_PRIORITY_CLOCK);
    RCC_stack_error(ERROR_BASE_RCC);
    // Init GPIOs.
    GPIO_init();
    EXTI_init();
    // Start independent watchdog.
#ifndef T82_ACCELERO_MODE_DEBUG
    iwdg_status = IWDG_init();
    IWDG_stack_error(ERROR_BASE_IWDG);
    IWDG_reload();
#endif
    // High speed oscillator.
    rcc_status = RCC_switch_to_hsi();
    RCC_stack_error(ERROR_BASE_RCC);
    // Calibrate clocks.
    rcc_status = RCC_calibrate_internal_clocks(NVIC_PRIORITY_CLOCK_CALIBRATION);
    RCC_stack_error(ERROR_BASE_RCC);
    // Init RTC.
    rtc_status = RTC_init(NULL, NVIC_PRIORITY_RTC);
    RTC_stack_error(ERROR_BASE_RTC);
    // Init delay timer.
    lptim_status = LPTIM_init(NVIC_PRIORITY_DELAY);
    LPTIM_stack_error(ERROR_BASE_LPTIM);
    // Init accelerometer.
    FXLS89XXXX_init();
}
/*******************************************************************/
int main(void) {
    // Init board.
    _T82_ACCELERO_init_context();
    _T82_ACCELERO_init_hw();
    // Sleep loop.
    while (1) {
        // Perform state machine.
        switch (t82_accelero_ctx.state) {
        case T82_ACCELERO_STATE_INIT:
            // Clear flags.
            t82_accelero_ctx.flags.all = 0;
            // T82 button.
            EXTI_configure_gpio(&GPIO_T82_BUTTON, GPIO_PULL_UP, EXTI_TRIGGER_FALLING_EDGE, &_T82_ACCELERO_button_irq_callback, NVIC_PRIORITY_BUTTON);
            EXTI_clear_gpio_flag(&GPIO_T82_BUTTON);
            EXTI_enable_gpio_interrupt(&GPIO_T82_BUTTON);
            // T82 LED.
            GPIO_configure(&GPIO_T82_LED, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
            GPIO_write(&GPIO_T82_LED, 0);
            // T82 alarm.
            GPIO_write(&GPIO_T82_ALARM, 1);
            GPIO_configure(&GPIO_T82_ALARM, GPIO_MODE_OUTPUT, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_UP);
            // Disable accelerometer.
            _T82_ACCELERO_disable_motion_irq();
            // Compute next state.
            t82_accelero_ctx.state = T82_ACCELERO_STATE_READY;
            break;
        case T82_ACCELERO_STATE_READY:
            // Enter stop mode.
            PWR_enter_deepsleep_mode(PWR_DEEPSLEEP_MODE_STOP);
            // Check button flag.
            if (t82_accelero_ctx.flags.button_pressed != 0) {
                // Get uptime and compute next state.
                t82_accelero_ctx.button_press_start_time = RTC_get_uptime_seconds();
                t82_accelero_ctx.state = T82_ACCELERO_STATE_ARMING_CONFIRMATION;
            }
            break;
        case T82_ACCELERO_STATE_ARMING_CONFIRMATION:
            // Check button state.
            if (GPIO_read(&GPIO_T82_BUTTON) == 0) {
                // Turn LED on during button press.
                GPIO_write(&GPIO_T82_LED, 1);
                // Check press time.
                if (RTC_get_uptime_seconds() >= (t82_accelero_ctx.button_press_start_time + T82_ACCELERO_ARMING_PRESS_DURATION_SECONDS)) {
                    // Start LED blinking.
                    GPIO_write(&GPIO_T82_LED, 0);
                    GPIO_configure(&GPIO_T82_LED, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
                    LPTIM_set_waveform(T82_ACCELERO_ARMING_LED_FREQUENCY_MHZ, T82_ACCELERO_ARMING_LED_DUTY_CYCLE_PERCENT);
                    // Arming confirmed.
                    t82_accelero_ctx.arming_start_time = RTC_get_uptime_seconds();
                    t82_accelero_ctx.state = T82_ACCELERO_STATE_ARMING;
                }
            }
            else {
                // Turn LED off.
                GPIO_write(&GPIO_T82_LED, 0);
                // Go back to ready.
                t82_accelero_ctx.flags.button_pressed = 0;
                t82_accelero_ctx.state = T82_ACCELERO_STATE_READY;
            }
            break;
        case T82_ACCELERO_STATE_ARMING:
            // Enter stop mode.
            PWR_enter_deepsleep_mode(PWR_DEEPSLEEP_MODE_STOP);
            // Check arming time.
            if (RTC_get_uptime_seconds() >= (t82_accelero_ctx.arming_start_time + T82_ACCELERO_ARMING_TIME_SECONDS)) {
                // Stop LED blinking.
                LPTIM_set_waveform(T82_ACCELERO_ARMING_LED_FREQUENCY_MHZ, 0);
                // Enable accelerometer.
                _T82_ACCELERO_enable_motion_irq();
                // Start LED blinking.
                LPTIM_set_waveform(T82_ACCELERO_ACTIVE_LED_FREQUENCY_MHZ, T82_ACCELERO_ACTIVE_LED_DUTY_CYCLE_PERCENT);
                // Go to active mode.
                t82_accelero_ctx.state = T82_ACCELERO_STATE_ACTIVE;
            }
            break;
        case T82_ACCELERO_STATE_ACTIVE:
            // Enter stop mode.
            PWR_enter_deepsleep_mode(PWR_DEEPSLEEP_MODE_STOP);
            // Check motion interrupt.
            if (t82_accelero_ctx.flags.motion_detected != 0) {
                // Stop LED blink.
                LPTIM_set_waveform(T82_ACCELERO_ARMING_LED_FREQUENCY_MHZ, 0);
                GPIO_configure(&GPIO_T82_LED, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
                GPIO_write(&GPIO_T82_LED, 0);
                // Wake-up talkie-walkie.
                GPIO_write(&GPIO_T82_ALARM, 0);
                LPTIM_delay_milliseconds(T82_ACCELERO_WAKE_UP_PULSE_MS, LPTIM_DELAY_MODE_STOP);
                GPIO_write(&GPIO_T82_ALARM, 1);
                LPTIM_delay_milliseconds(T82_ACCELERO_WAKE_UP_PULSE_MS, LPTIM_DELAY_MODE_STOP);
                // Send alarm.
                t82_accelero_ctx.state = T82_ACCELERO_STATE_ALARM;
            }
            break;
        case T82_ACCELERO_STATE_ALARM:
            // Start alarm.
            GPIO_write(&GPIO_T82_ALARM, 0);
            LPTIM_delay_milliseconds(T82_ACCELERO_ALARM_PULSE_MS, LPTIM_DELAY_MODE_STOP);
            GPIO_write(&GPIO_T82_ALARM, 1);
            // Re-start state machine.
            t82_accelero_ctx.state = T82_ACCELERO_STATE_INIT;
            break;
        default:
            t82_accelero_ctx.state = T82_ACCELERO_STATE_INIT;
            break;
        }
        IWDG_reload();
    }
    return 0;
}
