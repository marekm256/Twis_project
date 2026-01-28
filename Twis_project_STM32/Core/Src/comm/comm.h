#ifndef SRC_COMM_COMM_H_
#define SRC_COMM_COMM_H_

#include "main.h"
#include <stdint.h>

extern UART_HandleTypeDef huart1;

#define KEY_W       (1u << 0)
#define KEY_A       (1u << 1)
#define KEY_S       (1u << 2)
#define KEY_D       (1u << 3)
#define KEY_SPACE   (1u << 4)
#define KEY_E       (1u << 5)

extern volatile uint8_t g_keys_state;

void Comm_Init(void);
void Comm_Task(void);
void Comm_SendDistance(float dist_m);

#endif
