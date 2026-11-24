/* Generated: every model output kept alive in the TinyEngine arena (HWC int8). */
#pragma once
#include <stdint.h>
typedef struct { int h, w, c; const int8_t *data; float scale; int zero_point; } te_head_t;
#define TE_N_HEADS 9
#define TE_HEADS(buffer0) { \
    {16, 20, 2, (const int8_t *)&buffer0[47360], 0.0395918898f, 78}, \
    {16, 20, 8, (const int8_t *)&buffer0[16640], 0.0189117678f, -128}, \
    {16, 20, 20, (const int8_t *)&buffer0[0], 0.0247873161f, 1}, \
    {8, 10, 2, (const int8_t *)&buffer0[11200], 0.0356272049f, 67}, \
    {8, 10, 8, (const int8_t *)&buffer0[10560], 0.0192440189f, -128}, \
    {8, 10, 20, (const int8_t *)&buffer0[6400], 0.0263396166f, -1}, \
    {4, 5, 2, (const int8_t *)&buffer0[8560], 0.0238347966f, 127}, \
    {4, 5, 8, (const int8_t *)&buffer0[8400], 0.014089399f, -128}, \
    {4, 5, 20, (const int8_t *)&buffer0[8000], 0.0146276858f, -21}, \
}
