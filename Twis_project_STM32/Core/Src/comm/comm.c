#include "comm.h"
#include <string.h>


#define MSG_DISTANCE 0xD1
#define START_BYTE  0xAB
#define FRAME_LEN   3
#define TIMEOUT_MS  500

volatile uint8_t g_keys_state = 0;

static uint8_t rx_byte;
static uint8_t frame[FRAME_LEN];
static uint8_t idx = 0;
static uint32_t last_rx_ms = 0;

static void StartRx(void)
{
  (void)HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void Comm_Init(void)
{
  idx = 0;
  g_keys_state = 0;
  last_rx_ms = HAL_GetTick();
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
        last_rx_ms = HAL_GetTick();

        /* Uncomment for debugging communication
        if (g_keys_state) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		}
		*/
      }
    }
  }

  StartRx();
}

static uint8_t checksum_xor(const uint8_t *p, uint8_t n)
{
  uint8_t c = 0;
  for (uint8_t i = 0; i < n; i++) c ^= p[i];
  return c;
}

void Comm_SendDistance(float dist_m)
{
  uint8_t tx[7];
  tx[0] = START_BYTE;
  tx[1] = MSG_DISTANCE;

  // skopíruj float do bajtov (IEEE754)
  memcpy(&tx[2], &dist_m, sizeof(float));

  // checksum z bajtov [1..5] (typ + float)
  tx[6] = checksum_xor(&tx[1], 1 + 4);

  // blokujúco, na testovanie je to najjednoduchšie
  HAL_UART_Transmit(&huart1, tx, sizeof(tx), 50);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART1) return;
  idx = 0;
  StartRx();
}
