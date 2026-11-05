/* USB CDC (virtual COM port) glue for the camera streaming firmware.
 * Received bytes go into a small ring buffer read by the main loop.
 * Large buffers (camera frames) are sent in chunks with CDC_SendBlocking(). */
#include "usbd_cdc_if.h"
#include "stm32h7xx_hal.h"

#define APP_RX_DATA_SIZE  512
#define APP_TX_CHUNK      16384U   /* multiple of 64 bytes (FS bulk packet size) */
#define CMD_RING_SIZE     64

extern USBD_HandleTypeDef hUsbDeviceHS;

static uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];
static uint8_t UserTxBufferHS[64];

static volatile uint8_t ring[CMD_RING_SIZE];
static volatile uint16_t ring_head, ring_tail;
static volatile uint8_t host_connected;

static int8_t CDC_Init_HS(void);
static int8_t CDC_DeInit_HS(void);
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_HS(uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_HS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS = {
  CDC_Init_HS, CDC_DeInit_HS, CDC_Control_HS, CDC_Receive_HS, CDC_TransmitCplt_HS
};

/* Line coding is accepted but irrelevant: USB CDC runs at bus speed. */
static uint8_t line_coding[7] = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08}; /* 115200 8N1 */

static int8_t CDC_Init_HS(void)
{
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, UserTxBufferHS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, UserRxBufferHS);
  return USBD_OK;
}

static int8_t CDC_DeInit_HS(void)
{
  host_connected = 0;
  return USBD_OK;
}

static int8_t CDC_Control_HS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
  switch (cmd)
  {
  case CDC_SET_LINE_CODING:
    for (int i = 0; i < 7 && i < length; i++) line_coding[i] = pbuf[i];
    break;
  case CDC_GET_LINE_CODING:
    for (int i = 0; i < 7 && i < length; i++) pbuf[i] = line_coding[i];
    break;
  case CDC_SET_CONTROL_LINE_STATE:
  {
    USBD_SetupReqTypedef *req = (USBD_SetupReqTypedef *)pbuf;
    host_connected = (req->wValue & 0x0001U) ? 1U : 0U; /* DTR bit */
    break;
  }
  default:
    break;
  }
  return USBD_OK;
}

static int8_t CDC_Receive_HS(uint8_t *Buf, uint32_t *Len)
{
  for (uint32_t i = 0; i < *Len; i++)
  {
    uint16_t next = (uint16_t)((ring_head + 1U) % CMD_RING_SIZE);
    if (next != ring_tail) { ring[ring_head] = Buf[i]; ring_head = next; }
  }
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  return USBD_OK;
}

static int8_t CDC_TransmitCplt_HS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  (void)Buf; (void)Len; (void)epnum;
  return USBD_OK;
}

uint8_t CDC_Transmit_HS(uint8_t *Buf, uint32_t Len)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceHS.pClassData;
  if (hcdc == NULL) return USBD_FAIL;
  if (hcdc->TxState != 0U) return USBD_BUSY;
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, Buf, Len);
  return USBD_CDC_TransmitPacket(&hUsbDeviceHS);
}

int CDC_SendBlocking(const uint8_t *buf, uint32_t len, uint32_t timeout_ms)
{
  uint32_t sent = 0;
  uint32_t t0 = HAL_GetTick();
  while (sent < len)
  {
    uint32_t n = len - sent;
    if (n > APP_TX_CHUNK) n = APP_TX_CHUNK;
    uint8_t r = CDC_Transmit_HS((uint8_t *)&buf[sent], n);
    if (r == USBD_OK) { sent += n; t0 = HAL_GetTick(); }
    else if (r != USBD_BUSY) return -1;
    if (HAL_GetTick() - t0 > timeout_ms) return -1;
  }
  /* wait for the last chunk to leave so the caller may reuse the buffer */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceHS.pClassData;
  t0 = HAL_GetTick();
  while (hcdc && hcdc->TxState != 0U)
    if (HAL_GetTick() - t0 > timeout_ms) return -1;
  return 0;
}

uint8_t CDC_HostConnected(void) { return host_connected; }

int CDC_ReadByte(void)
{
  if (ring_tail == ring_head) return -1;
  uint8_t c = ring[ring_tail];
  ring_tail = (uint16_t)((ring_tail + 1U) % CMD_RING_SIZE);
  return c;
}
