#!/usr/bin/env bash
# Compile + link TinyEngine-generated code for the STM32H723 (Cortex-M7) to get
# real flash/RAM numbers. Usage: build_check_m7.sh <gen_dir> <tinyengine_dir>
set -e
GEN=$(realpath "$1"); TE=$(realpath "$2")/TinyEngine
OUT=$GEN/m7build; rm -rf "$OUT"; mkdir -p "$OUT"
# put the activation arena in AXI SRAM (DTCM is only 128 KB)
sed -i "s/^static signed char buffer\[/__attribute__((section(\".axi_sram\"), aligned(32))) static signed char buffer[/" $GEN/codegen/Include/genModel.h
CMSIS=$TE/third_party/CMSIS/CMSIS
INC="-I$GEN/codegen/Include -I$TE/include -I$CMSIS/NN/Include -I$CMSIS/DSP/Include -I$CMSIS/Core/Include"
CFLAGS="-include arm_nnsupportfunctions.h -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -O3 -ffunction-sections -fdata-sections -DARM_MATH_DSP -DARM_MATH_LOOPUNROLL -w"
SRCS="$(ls $GEN/codegen/Source/*.c) $(find $TE/src/kernels/int_forward_op $TE/src/kernels/fp_requantize_op -name '*.c')"
# CMSIS-NN support functions used by TinyEngine kernels
SRCS="$SRCS $(find $CMSIS/NN/Source -name '*.c' 2>/dev/null)"
cat > $OUT/stub.c <<'EOF'
#include <stdint.h>
signed char *getInput(void); signed char *getOutput(void); void invoke_inf(void);
extern uint32_t _estack;
volatile int8_t sink;
void Reset_Handler(void) { signed char *in = getInput(); for (int i = 0; i < 16; i++) in[i] = i;
  invoke_inf(); sink = getOutput()[0]; for (;;) {} }
__attribute__((section(".isr_vector"))) const void *vectors[2] = { &_estack, (void *)Reset_Handler };
EOF
objs=""
fail=0
for f in $SRCS $OUT/stub.c; do o=$OUT/$(basename ${f%.c}).o
  if arm-none-eabi-gcc $CFLAGS $INC -c "$f" -o "$o" 2>>$OUT/errors.log; then objs="$objs $o"; else fail=$((fail+1)); fi
done
echo "compile failures (unused files are OK if link succeeds): $fail"
arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -nostartfiles -specs=nano.specs -specs=nosys.specs \
  -T "$3" -Wl,--gc-sections -Wl,--print-memory-usage $objs -o $OUT/check.elf -lm 2>&1 | grep -E "FLASH|DTCM|RAM_D1|error|undefined" | head -20
arm-none-eabi-size -A $OUT/check.elf | grep -E "^\.(text|rodata|data|bss|axi_sram)" || true
