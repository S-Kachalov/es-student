#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PI_TERMS_DEFAULT 1000000

typedef enum {
  NOT_STARTED,
  RUNNING,
  DONE,
} pi_state_t;

pi_state_t pi_task_get_state(void);
bool pi_task_start(int n);
bool pi_task_get_result(double *result, uint64_t *time_spent);
void pi_task_handle(void);
