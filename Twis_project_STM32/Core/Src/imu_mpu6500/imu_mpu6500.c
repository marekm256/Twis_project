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
#include <math.h>

extern I2C_HandleTypeDef hi2c1;

/* ===== MPU6500 fixed I2C address =====
 * AD0=0 -> 0x68
 */
#define MPU6500_ADDR7          (0x68u)
#define MPU6500_ADDR8          ((uint16_t)(MPU6500_ADDR7 << 1))

/* ===== Registers ===== */
#define REG_WHO_AM_I           (0x75u)
#define REG_PWR_MGMT_1         (0x6Bu)

#define REG_SMPLRT_DIV         (0x19u)
#define REG_CONFIG             (0x1Au)
#define REG_GYRO_CONFIG        (0x1Bu)
#define REG_ACCEL_CONFIG       (0x1Cu)
#define REG_ACCEL_CONFIG2      (0x1Du)

#define REG_ACCEL_XOUT_H       (0x3Bu)   // start of 14B burst: accel(6), temp(2), gyro(6)

/* ===== Timeouts ===== */
#define I2C_TIMEOUT_MS         (50u)

static bool  s_inited = false;

#define COMP_ALPHA  (0.98f)
static float    s_roll_deg  = 0.0f;
static float    s_pitch_deg = 0.0f;
static uint32_t s_last_ms   = 0;

/* Gyro bias (deg/s) */
static float s_gx_bias = 0.0f;
static float s_gy_bias = 0.0f;
static float s_gz_bias = 0.0f;

/* Selected full-scale ranges (must match init config below) */
static float s_gyro_lsb_per_dps  = 16.4f;    // for ±2000 dps
static float s_accel_lsb_per_g   = 8192.0f;  // for ±4 g

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

static HAL_StatusTypeDef imu_read_bytes(uint8_t reg, uint8_t *buf, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c1,
                          MPU6500_ADDR8,
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          buf,
                          len,
                          I2C_TIMEOUT_MS);
}

static bool whoami_ok(uint8_t w)
{
  // MPU6500 typicky 0x70, niektoré moduly/varianty môžu vrátiť aj 0x71/0x68
  return (w == 0x70u) || (w == 0x71u) || (w == 0x68u);
}

/* Read burst raw (internal helper used for calibration too) */
static HAL_StatusTypeDef imu_read_raw_burst(int16_t* ax, int16_t* ay, int16_t* az,
                                           int16_t* gx, int16_t* gy, int16_t* gz)
{
  uint8_t b[14];
  HAL_StatusTypeDef st = imu_read_bytes(REG_ACCEL_XOUT_H, b, sizeof(b));
  if (st != HAL_OK) return st;

  *ax = (int16_t)((b[0] << 8) | b[1]);
  *ay = (int16_t)((b[2] << 8) | b[3]);
  *az = (int16_t)((b[4] << 8) | b[5]);

  *gx = (int16_t)((b[8]  << 8) | b[9]);
  *gy = (int16_t)((b[10] << 8) | b[11]);
  *gz = (int16_t)((b[12] << 8) | b[13]);

  return HAL_OK;
}

void IMU_Init(void)
{
  s_inited = false;

  // Wake up (clear SLEEP) - môžeš neskôr prejsť na 0x01 (PLL), ale nechávam tvoj štýl
  (void)imu_write_u8(REG_PWR_MGMT_1, 0x00u);
  HAL_Delay(10);

  // Verify WHO_AM_I
  uint8_t w = 0;
  if (imu_read_u8(REG_WHO_AM_I, &w) != HAL_OK) return;
  if (!whoami_ok(w)) return;

  // ===== Configure sensor =====
  (void)imu_write_u8(REG_CONFIG, 0x03u);     // gyro DLPF ~44Hz
  (void)imu_write_u8(REG_SMPLRT_DIV, 4u);    // ~200Hz when DLPF enabled

  (void)imu_write_u8(REG_GYRO_CONFIG, (uint8_t)(3u << 3));  // ±2000 dps
  s_gyro_lsb_per_dps = 16.4f;

  (void)imu_write_u8(REG_ACCEL_CONFIG, (uint8_t)(1u << 3)); // ±4g
  s_accel_lsb_per_g = 8192.0f;

  (void)imu_write_u8(REG_ACCEL_CONFIG2, 0x03u); // accel DLPF ~44Hz

  // ===== Gyro bias calibration (keep the board still!) =====
  s_gx_bias = 0.0f;
  s_gy_bias = 0.0f;
  s_gz_bias = 0.0f;

  const int N = 200;           // ~200 samples
  const uint32_t sample_delay_ms = 5; // ~1s total
  int good = 0;

  for (int i = 0; i < N; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    if (imu_read_raw_burst(&ax, &ay, &az, &gx, &gy, &gz) == HAL_OK) {
      s_gx_bias += (float)gx / s_gyro_lsb_per_dps;
      s_gy_bias += (float)gy / s_gyro_lsb_per_dps;
      s_gz_bias += (float)gz / s_gyro_lsb_per_dps;
      good++;
    }
    HAL_Delay(sample_delay_ms);
  }

  if (good > 0) {
    s_gx_bias /= (float)good;
    s_gy_bias /= (float)good;
    s_gz_bias /= (float)good;
  }

  // ===== Initialize roll/pitch using accelerometer angle (no jump at start) =====
  {
    int16_t ax, ay, az, gx, gy, gz;
    if (imu_read_raw_burst(&ax, &ay, &az, &gx, &gy, &gz) == HAL_OK) {
      float ax_g = (float)ax / s_accel_lsb_per_g;
      float ay_g = (float)ay / s_accel_lsb_per_g;
      float az_g = (float)az / s_accel_lsb_per_g;

      const float RAD2DEG = 57.2957795f;
      s_roll_deg  = atan2f(ay_g, az_g) * RAD2DEG;

      float denom = sqrtf(ay_g*ay_g + az_g*az_g);
      if (denom < 1e-6f) denom = 1e-6f;
      s_pitch_deg = atan2f(-ax_g, denom) * RAD2DEG;
    } else {
      s_roll_deg = 0.0f;
      s_pitch_deg = 0.0f;
    }
  }

  s_last_ms = HAL_GetTick();
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

HAL_StatusTypeDef IMU_ReadRaw(IMU_Raw *out)
{
  if (!out) return HAL_ERROR;
  if (!s_inited) return HAL_ERROR;

  if (IMU_Update() < 0.5f) return HAL_ERROR;

  int16_t ax, ay, az, gx, gy, gz;
  HAL_StatusTypeDef st = imu_read_raw_burst(&ax, &ay, &az, &gx, &gy, &gz);
  if (st != HAL_OK) return st;

  out->ax = ax; out->ay = ay; out->az = az;
  out->gx = gx; out->gy = gy; out->gz = gz;

  return HAL_OK;
}

HAL_StatusTypeDef IMU_ReadData(IMU_Data *out)
{
  if (!out) return HAL_ERROR;

  IMU_Raw r;
  HAL_StatusTypeDef st = IMU_ReadRaw(&r);
  if (st != HAL_OK) return st;

  out->ax_g = (float)r.ax / s_accel_lsb_per_g;
  out->ay_g = (float)r.ay / s_accel_lsb_per_g;
  out->az_g = (float)r.az / s_accel_lsb_per_g;

  // gyro in dps + remove bias
  out->gx_dps = ((float)r.gx / s_gyro_lsb_per_dps) - s_gx_bias;
  out->gy_dps = ((float)r.gy / s_gyro_lsb_per_dps) - s_gy_bias;
  out->gz_dps = ((float)r.gz / s_gyro_lsb_per_dps) - s_gz_bias;

  // --- dt (seconds) ---
  uint32_t now = HAL_GetTick();
  float dt = (now - s_last_ms) / 1000.0f;
  if (dt <= 0.0f) dt = 0.001f;
  s_last_ms = now;

  // --- angles from accelerometer (deg) ---
  const float RAD2DEG = 57.2957795f;

  float roll_acc = atan2f(out->ay_g, out->az_g) * RAD2DEG;

  float denom = sqrtf(out->ay_g*out->ay_g + out->az_g*out->az_g);
  if (denom < 1e-6f) denom = 1e-6f;
  float pitch_acc = atan2f(-out->ax_g, denom) * RAD2DEG;

  // --- integrate gyro (deg) ---
  float roll_gyro  = s_roll_deg  + out->gx_dps * dt;
  float pitch_gyro = s_pitch_deg + out->gy_dps * dt;

  // --- complementary filter ---
  s_roll_deg  = COMP_ALPHA * roll_gyro  + (1.0f - COMP_ALPHA) * roll_acc;
  s_pitch_deg = COMP_ALPHA * pitch_gyro + (1.0f - COMP_ALPHA) * pitch_acc;

  out->roll_deg  = s_roll_deg;
  out->pitch_deg = s_pitch_deg;

  return HAL_OK;
}
