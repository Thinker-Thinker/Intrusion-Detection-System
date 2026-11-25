/* PC test build: the bits of main.h / HAL that door.c and admin.c use */
#pragma once
#include <stdint.h>
uint32_t HAL_GetTick(void);
