/* USB CDC (virtual COM port) glue for the camera streaming firmware. */
#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_cdc.h"
#include <stdint.h>

extern USBD_CDC_ItfTypeDef USBD_Interface_fops_HS;

/* Non-blocking start of a transmit; returns USBD_BUSY while a previous one runs. */
uint8_t CDC_Transmit_HS(uint8_t *Buf, uint32_t Len);

/* Blocking send of an arbitrary-length buffer, split into chunks.
 * Returns 0 on success, -1 on timeout (e.g. host not reading). */
int CDC_SendBlocking(const uint8_t *buf, uint32_t len, uint32_t timeout_ms);

/* 1 once the host has opened the port (DTR asserted). */
uint8_t CDC_HostConnected(void);

/* Pop one received command byte; returns -1 when nothing is waiting. */
int CDC_ReadByte(void);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_H__ */
