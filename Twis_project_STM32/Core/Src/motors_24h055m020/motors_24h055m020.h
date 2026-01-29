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
void Motors_Brake(bool enable);   // enable=true -> BRAKE ON (pull-down)
void Motors_Control(uint8_t keys_state);

#endif /* SRC_MOTORS_24H055M020_MOTORS_24H055M020_H_ */
