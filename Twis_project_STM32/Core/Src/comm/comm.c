#include "comm.h"

extern UART_HandleTypeDef huart1;

#define COMM_START_BYTE    0xAA
#define COMM_FRAME_LEN     4
#define COMM_TIMEOUT_MS    500

static uint8_t  s_rx_byte;
static uint8_t  s_frame[COMM_FRAME_LEN];
static uint8_t  s_idx = 0;
static uint32_t s_last_rx_ms = 0;

static void Comm_StartRxIT(void)
{
  (void)HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
}

static void Comm_OnValidCmd(uint8_t cmd)
{
  // 1=W,2=A,3=S,4=D,5=STOP
  if (cmd >= 1 && cmd <= 4) {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3); // LED1 PB3 (D13)
  }
}

void Comm_Init(void)
{
  s_idx = 0;
  s_last_rx_ms = HAL_GetTick();
  Comm_StartRxIT();
}

void Comm_Task(void)
{
  // zatial nič nerobíme, len držíme miesto na timeout (neskôr STOP)
  if ((HAL_GetTick() - s_last_rx_ms) > COMM_TIMEOUT_MS) {
    s_last_rx_ms = HAL_GetTick();
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART1) return;

  uint8_t b = s_rx_byte;

  // Frame: AA CMD VAL CHK (CHK = CMD XOR VAL)
  if (s_idx == 0) {
    if (b == COMM_START_BYTE) {
      s_frame[s_idx++] = b;
    }
  } else {
    s_frame[s_idx++] = b;

    if (s_idx >= COMM_FRAME_LEN) {
      uint8_t cmd  = s_frame[1];
      uint8_t val  = s_frame[2];
      uint8_t chk  = s_frame[3];
      uint8_t calc = (uint8_t)(cmd ^ val);

      s_idx = 0;

      if (chk == calc && cmd >= 1 && cmd <= 5) {
        s_last_rx_ms = HAL_GetTick();
        Comm_OnValidCmd(cmd);
      }
    }
  }

  Comm_StartRxIT();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART1) return;
  s_idx = 0;
  Comm_StartRxIT();
}
