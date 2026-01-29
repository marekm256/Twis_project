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

void  IMU_Init(void);

/**
 * @brief  Ping IMU via WHO_AM_I.
 * @return 1.0f = OK, 0.0f = fail
 */
float IMU_Update(void);

// ---- NEW: data reading ----
typedef struct {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
} IMU_Raw;

typedef struct {
  float ax_g, ay_g, az_g;
  float gx_dps, gy_dps, gz_dps;
  float roll_deg, pitch_deg;
} IMU_Data;

/**
 * @brief Read raw accel+gyro registers.
 * @return HAL_OK / error code
 */
HAL_StatusTypeDef IMU_ReadRaw(IMU_Raw *out);

/**
 * @brief Read and convert to physical units (g, dps, Â°C).
 * @return HAL_OK / error code
 */
HAL_StatusTypeDef IMU_ReadData(IMU_Data *out);

#ifdef __cplusplus
}
#endif

#endif /* SRC_IMU_MPU6500_IMU_MPU6500_H_ */
