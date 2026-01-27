#include "comm.h"

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

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART1) return;
  idx = 0;
  StartRx();
}
