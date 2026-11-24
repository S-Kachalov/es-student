#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  NOT_STARTED,
  RUNNING,
  BUSY,
} pi_state_t;

pi_state_t pi_task_get_state(void);
bool pi_task_start(int n);
bool pi_task_get_result(double *result, uint64_t *time_spent);
void pi_task_handle(void);
