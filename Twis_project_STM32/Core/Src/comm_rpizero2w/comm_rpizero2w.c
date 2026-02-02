#include "comm_rpizero2w.h"
#include <string.h>

#define START_BYTE  0xAB
#define FRAME_LEN   3
#define MSG_TELEM   0xD2  

volatile uint8_t g_keys_state = 0;

static uint8_t rx_byte;
static uint8_t frame[FRAME_LEN];
static uint8_t idx = 0;

static void StartRx(void)
{
  (void)HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

static uint8_t checksum_xor(const uint8_t *p, uint8_t n)
{
  uint8_t c = 0;
  for (uint8_t i = 0; i < n; i++) c ^= p[i];
  return c;
}

void Comm_Init(void)
{
  idx = 0;
  g_keys_state = 0;
  StartRx();
}

void Comm_Task(void)
{
  
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART1) return;

  uint8_t b = rx_byte;

  if (idx == 0) {
    if (b == START_BYTE) {
      frame[idx++] = b;
    }
  } else {
    frame[idx++] = b;

    if (idx >= FRAME_LEN) {
      uint8_t state = frame[1] & 0x3F;
      uint8_t chk   = frame[2];
      idx = 0;

      if (((uint8_t)(state ^ 0xFF)) == chk) {
        g_keys_state = state;

        if (g_keys_state) {
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
        } else {
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        }
      }
    }
  }

  StartRx();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART1) return;
  idx = 0;
  StartRx();
}

void Comm_SendTelem11(const float v[11])
{
  
  uint8_t tx[1 + 1 + 1 + 44 + 1];
  tx[0] = START_BYTE;
  tx[1] = MSG_TELEM;
  tx[2] = 44; // 11 floats

  memcpy(&tx[3], v, 44);

  
  tx[3 + 44] = checksum_xor(&tx[1], 1 + 1 + 44);

  HAL_UART_Transmit(&huart1, tx, sizeof(tx), 50);
}
