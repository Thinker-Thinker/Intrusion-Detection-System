#pragma once
#include <stdint.h>
int CDC_HostConnected(void);
int CDC_SendBlocking(const uint8_t *p, uint32_t n, uint32_t t);
int CDC_ReadByte(void);
