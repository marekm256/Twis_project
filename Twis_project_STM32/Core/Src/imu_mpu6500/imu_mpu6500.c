/*
 * imu_mpu6050.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

// imu_mpu6050.c
#include "imu_mpu6500.h"
#include "i2c.h"
#include "stm32f3xx_hal.h"
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c1;

/* ===== MPU6500 fixed I2C address =====
 * AD0=0 -> 0x68
 */
#define MPU6500_ADDR7          (0x68u)
#define MPU6500_ADDR8          ((uint16_t)(MPU6500_ADDR7 << 1))

/* ===== Registers ===== */
#define REG_WHO_AM_I           (0x75u)
#define REG_PWR_MGMT_1         (0x6Bu)

/* ===== Timeouts ===== */
#define I2C_TIMEOUT_MS         (50u)

static bool s_inited = false;

static HAL_StatusTypeDef imu_read_u8(uint8_t reg, uint8_t *val)
{
  return HAL_I2C_Mem_Read(&hi2c1,
                          MPU6500_ADDR8,
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          val,
                          1,
                          I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef imu_write_u8(uint8_t reg, uint8_t val)
{
  return HAL_I2C_Mem_Write(&hi2c1,
                           MPU6500_ADDR8,
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           &val,
                           1,
                           I2C_TIMEOUT_MS);
}

static bool whoami_ok(uint8_t w)
{
  // MPU6500 typicky 0x70, niektoré moduly/varianty môžu vrátiť aj 0x71/0x68
  return (w == 0x70u) || (w == 0x71u) || (w == 0x68u);
}

void IMU_Init(void)
{
  s_inited = false;

  // Wake up (clear SLEEP)
  (void)imu_write_u8(REG_PWR_MGMT_1, 0x00u);
  HAL_Delay(10);

  // Verify WHO_AM_I
  uint8_t w = 0;
  if (imu_read_u8(REG_WHO_AM_I, &w) != HAL_OK) {
    return;
  }
  if (!whoami_ok(w)) {
    return;
  }

  s_inited = true;
}

float IMU_Update(void)
{
  if (!s_inited) return 0.0f;

  uint8_t w = 0;
  if (imu_read_u8(REG_WHO_AM_I, &w) != HAL_OK) return 0.0f;
  if (!whoami_ok(w)) return 0.0f;

  return 1.0f;
}
