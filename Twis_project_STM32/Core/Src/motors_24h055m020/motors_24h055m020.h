/*
 * motor_control.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_MOTORS_24H055M020_MOTORS_24H055M020_H_
#define SRC_MOTORS_24H055M020_MOTORS_24H055M020_H_

#include <stdint.h>
#include <stdbool.h>

void Motors_Init(void);
void Motors_Speed_inPercent(float left_pct, float right_pct); //  Signed percent: -100..+100
void Motors_SetEnable(bool enable);   // enable = true -> L_EN/R_EN HIGH
void Motors_Control(uint8_t keys_state, float ay, float gyro_y, float d_avg_cm);

#endif /* SRC_MOTORS_24H055M020_MOTORS_24H055M020_H_ */
