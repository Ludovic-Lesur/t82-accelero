/*
 * t82_accelero.h
 *
 *  Created on: 27 apr. 2026
 *      Author: Ludo
 */

#ifndef __T82_ACCELERO_FLAGS_H__
#define __T82_ACCELERO_FLAGS_H__

/*** Board modes ***/

//#define T82_ACCELERO_MODE_DEBUG

/*** Board options ***/

#define T82_ACCELERO_ARMING_PRESS_DURATION_SECONDS  5

#define T82_ACCELERO_ARMING_TIME_SECONDS            30
#define T82_ACCELERO_ARMING_LED_PULSE_MS            200
#define T82_ACCELERO_ARMING_LED_PERIOD_MS           1000

#define T82_ACCELERO_ACTIVE_LED_PULSE_MS            200
#define T82_ACCELERO_ACTIVE_LED_PERIOD_MS           3000

#define T82_ACCELERO_MOTION_IRQ_COUNT_THRESHOLD     3

#endif /* __T82_ACCELERO_FLAGS_H__ */
