/*
 * imu_mpu6050.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_IMU_MPU6500_IMU_MPU6500_H_
#define SRC_IMU_MPU6500_IMU_MPU6500_H_

#include <stdint.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef IMU_Init(void);


uint8_t IMU_IsReady(void);
uint8_t IMU_GetWhoAmI(void);


typedef struct {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  int16_t temp;
} IMU_Raw;


typedef struct {
  float ax_g, ay_g, az_g;
  float gx_dps, gy_dps, gz_dps;
  float temp_c;
  float roll_degmean, roll_deg;
} IMU_Data;

HAL_StatusTypeDef IMU_ReadRaw(IMU_Raw *out);
HAL_StatusTypeDef IMU_ReadData(IMU_Data *out);

#ifdef __cplusplus
}
#endif

#endif /* SRC_IMU_MPU6500_IMU_MPU6500_H_ */
