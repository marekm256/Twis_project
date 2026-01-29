/*
 * imu_mpu6050.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_IMU_MPU6500_IMU_MPU6500_H_
#define SRC_IMU_MPU6500_IMU_MPU6500_H_


#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void  IMU_Init(void);

/**
 * @brief  Ping IMU via WHO_AM_I.
 * @return 1.0f = OK, 0.0f = fail
 */
float IMU_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_IMU_MPU6500_IMU_MPU6500_H_ */
