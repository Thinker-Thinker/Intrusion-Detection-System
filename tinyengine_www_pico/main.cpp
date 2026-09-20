/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"

#include "testing_data/golden_data.h"
#include "testing_data/images.h"
extern "C"
{
#include "genNN.h"
#include "tinyengine_function.h"
}

#define RES_W 80
#define RES_H 80
#define OUTPUT_CH 2
uint16_t *RGBbuf;
signed char out_int[OUTPUT_CH];

float labels[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

void invoke_new_weights_givenimg(signed char *out_int8)
{
    invoke(labels);
    signed char *output = (signed char *)getOutput();
    for (int i = 0; i < OUTPUT_CH; i++)
        out_int8[i] = output[i];
}

int main()
{
    stdio_init_all();
    char buf[150];
    bool should_person = true;
    while (1)
    {
        should_person != should_person;
        uint32_t start, end;
        signed char *input = getInput();
        for (int i = 0; i < RES_W * RES_W * 3; i++)
        {
            if (should_person)
                input[i] = person[i]; // Image of person
            else
                input[i] = no_person[i]; // Image of non-person
        }
        RGBbuf = (uint16_t *)&input[80 * 80 * 4];
        int t_mode = 0;

        start = time_us_32();

        for (int i = 0; i < RES_W; i++)
        {
            for (int j = 0; j < RES_W; j++)
            {
                uint8_t red = (int32_t)input[(80 * i + j) * 3] + 128;
                uint8_t green = (int32_t)input[(80 * i + j) * 3 + 1] + 128;
                uint8_t blue = (int32_t)input[(80 * i + j) * 3 + 2] + 128;

                uint16_t b = (blue >> 3) & 0x1f;
                uint16_t g = ((green >> 2) & 0x3f) << 5;
                uint16_t r = ((red >> 3) & 0x1f) << 11;

                RGBbuf[j + RES_W * i] = (uint16_t)(r | g | b);
            }
        }
        // loadRGB565LCD(10, 10, RES_W, RES_W, RGBbuf, 3);

        invoke_new_weights_givenimg(out_int);
        int person = 0;
        if (out_int[0] > out_int[1])
        {
            person = 0;
        }
        else
        {
            person = 1;
        }
        end = time_us_32();
        printf("Inference with person: %d\n", person);
        // detectResponse(person, end - start, t_mode, 0, 0);
    }
}
