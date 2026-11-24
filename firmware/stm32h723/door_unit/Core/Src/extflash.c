/* External W25Q64 flash: memory-mapped reads for the recognition weights, erase/program for updates. */
#include "main.h"
#include "octospi.h"
#include "w25qxx_ospi.h"
#include "extflash.h"
#include <string.h>

extern const uint8_t __extflash_start[], __extflash_end[];   /* from the linker script */

static int ok, mapped, writing, rec_valid;
static uint32_t jedec;

static uint32_t crc_table[256];
uint32_t crc32_buf(uint32_t crc, const uint8_t *p, uint32_t n)
{
  if (!crc_table[1])
    for (uint32_t i = 0; i < 256; i++) {
      uint32_t c = i;
      for (int k = 0; k < 8; k++) c = (c & 1U) ? (0xEDB88320U ^ (c >> 1)) : (c >> 1);
      crc_table[i] = c;
    }
  crc = ~crc;
  while (n--) crc = crc_table[(crc ^ *p++) & 0xFFU] ^ (crc >> 8);
  return ~crc;
}

static void drop_cache(void)
{
  SCB_InvalidateDCache_by_Addr((uint32_t *)XF_BASE, 0x800000);
  SCB_InvalidateICache();
}

static int map_on(void)
{
  if (mapped) return 0;
  if (OSPI_NOR_EnableSTRMemoryMappedMode() != W25Qxx_OK) return -1;
  mapped = 1;
  drop_cache();
  return 0;
}

static int map_off(void)
{
  if (!mapped) return 0;
  if (OSPI_NOR_DisableSTRMemoryMappedMode() != W25Qxx_OK) return -1;
  mapped = 0;
  return 0;
}

uint32_t xf_rec_size(void) { return (uint32_t)(__extflash_end - __extflash_start); }
int xf_ok(void) { return ok; }
int xf_rec_valid(void) { return rec_valid; }
int xf_writing(void) { return writing; }
uint32_t xf_jedec_id(void) { return jedec; }

int xf_rec_check(void)
{
  rec_valid = 0;
  if (!ok || !mapped) return 0;
  const xf_header_t *h = (const xf_header_t *)(XF_BASE + XF_HDR_OFF);
  if (h->magic != XF_MAGIC_REC || h->length != xf_rec_size() || h->length == 0) return 0;
  rec_valid = crc32_buf(0, (const uint8_t *)XF_BASE, h->length) == h->crc;
  return rec_valid;
}

int xf_init(void)
{
  MX_OCTOSPI1_Init();
  if (OSPI_NOR_Init() != W25Qxx_OK) return -1;
  jedec = OSPI_NOR_ID;
  ok = 1;
  if (map_on() != 0) { ok = 0; return -1; }
  xf_rec_check();
  return 0;
}

int xf_write_begin(void)
{
  if (!ok) return -1;
  rec_valid = 0;                         /* recognition must not read the flash while it is written */
  if (map_off() != 0) return -1;
  writing = 1;
  return 0;
}

int xf_erase_blocks(uint32_t off, uint32_t len)
{
  if (!writing) return -1;
  for (uint32_t a = off & ~(XF_BLOCK - 1); a < off + len; a += XF_BLOCK)
    if (OSPI_NOR_BlockErase(a) != W25Qxx_OK) return -1;
  return 0;
}

int xf_program(uint32_t off, const uint8_t *data, uint32_t len)
{
  if (!writing) return -1;
  /* the driver takes a non-const pointer and at most 65535 bytes per call */
  while (len) {
    uint32_t n = len > 32768 ? 32768 : len;
    if (OSPI_NOR_WriteNoCheck((uint8_t *)data, off, n) != W25Qxx_OK) return -1;
    data += n; off += n; len -= n;
  }
  return 0;
}

int xf_write_end(void)
{
  writing = 0;
  if (map_on() != 0) return -1;
  xf_rec_check();
  return 0;
}
