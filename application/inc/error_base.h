/*
 * error.h
 *
 *  Created on: 27 apr. 2026
 *      Author: Ludo
 */

#ifndef __ERROR_BASE_H__
#define __ERROR_BASE_H__

// Peripherals.
#include "i2c.h"
#include "iwdg.h"
#include "lptim.h"
#include "rcc.h"
#include "rtc.h"
#include "tim.h"
// Utils.
#include "error.h"
// Components.
#include "fxls89xxxx.h"

/*** ERROR BASE structures ***/

/*!******************************************************************
 * \enum ERROR_base_t
 * \brief Board error bases.
 *******************************************************************/
typedef enum {
    SUCCESS = 0,
    // Peripherals.
    ERROR_BASE_IWDG = ERROR_BASE_STEP,
    ERROR_BASE_LPTIM = (ERROR_BASE_IWDG + IWDG_ERROR_BASE_LAST),
    ERROR_BASE_RCC = (ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST),
    ERROR_BASE_RTC = (ERROR_BASE_RCC + RCC_ERROR_BASE_LAST),
    // Components.
    ERROR_BASE_FXLS8974CF = (ERROR_BASE_RTC + RTC_ERROR_BASE_LAST),
    // Last base value.
    ERROR_BASE_LAST = (ERROR_BASE_FXLS8974CF + FXLS89XXXX_ERROR_BASE_LAST)
} ERROR_base_t;

#endif /* __ERROR_BASE_H__ */
