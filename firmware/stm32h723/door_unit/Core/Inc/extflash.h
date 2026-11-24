/* External 8 MB W25Q64 flash on the WeAct board (OCTOSPI1, quad SPI), memory-mapped at 0x90000000.
 *
 * Layout:
 *   0x000000 .. 0x7BFFFF   recognition model weights (the linker's .extflash section, rec_weights.bin)
 *   0x7E0000 .. 0x7EFFFF   enrolled people (face templates)
 *   0x7F0000 .. 0x7FFFFF   header describing the weights (magic, length, CRC-32)
 *
 * The weights are written from the PC over USB (tools/door.py flash-model); the board checks the CRC
 * at every start and only runs face recognition when it matches. */
#ifndef EXTFLASH_H
#define EXTFLASH_H
#include <stdint.h>

#define XF_BASE        0x90000000u
#define XF_TPL_OFF     0x7E0000u
#define XF_HDR_OFF     0x7F0000u
#define XF_BLOCK       0x10000u

typedef struct { uint32_t magic, length, crc, reserved; } xf_header_t;
#define XF_MAGIC_REC 0x57434552u          /* "RECW" */

int  xf_init(void);                        /* OSPI + flash reset, quad enable, memory-mapped; 0 = ok */
int  xf_ok(void);
uint32_t xf_jedec_id(void);
/* Recognition weights: expected size from the linker; 1 when the stored copy has a valid header and CRC. */
uint32_t xf_rec_size(void);
int  xf_rec_valid(void);
int  xf_rec_check(void);                   /* recompute (slow, ~30 ms): sets and returns xf_rec_valid() */

/* Writing: leaves memory-mapped mode until xf_write_end(). Offsets are from the start of the chip. */
int  xf_write_begin(void);
int  xf_erase_blocks(uint32_t off, uint32_t len);       /* 64 KB blocks covering [off, off+len) */
int  xf_program(uint32_t off, const uint8_t *data, uint32_t len);
int  xf_write_end(void);                   /* back to memory-mapped, caches dropped */
int  xf_writing(void);

uint32_t crc32_buf(uint32_t crc, const uint8_t *p, uint32_t n);   /* zlib CRC-32, start with crc = 0 */
#endif
