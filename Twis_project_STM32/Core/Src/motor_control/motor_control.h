/*
 * motor_control.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_
#define SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>

void Motors_Init(void);

/* enable=true  -> ramp na target_freq_hz (napr. 50)
   enable=false -> ramp na 0 a vypne PWM
   ramp_ms definuje dobu rampy (0 = skok) */
void Motors_Update(bool enable, uint32_t target_freq_hz, uint32_t ramp_ms);

#endif /* SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_ */
