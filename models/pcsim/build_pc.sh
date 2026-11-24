#!/usr/bin/env bash
# Build TinyEngine-generated model code + kernels for the PC (checking against TFLite).
# Usage: build_pc.sh <gen_dir> <tinyengine_clone> <harness.c> <out_exe> [extra .c files...]
set -e
GEN=$(realpath "$1"); TE=$(realpath "$2")/TinyEngine; H=$(realpath "$3"); OUT=$(realpath -m "$4"); shift 4
CM=$TE/third_party/CMSIS/CMSIS; SIM=$(dirname "$(realpath "$0")")
B=$(mktemp -d); mkdir -p $B/fakecore; printf '#include "arm_shim.h"\n' > $B/fakecore/cmsis_compiler.h; cp $B/fakecore/cmsis_compiler.h $B/fakecore/core_cm7.h
INC="-I$SIM -I$B/fakecore -I$GEN/codegen/Include -I$TE/include -I$CM/NN/Include -I$CM/DSP/Include"
F="-O2 -w -ffunction-sections -fdata-sections -include arm_shim.h -include te_protos.h -DARM_MATH_DSP -D__ARM_FEATURE_DSP=1 -DARM_MATH_LOOPUNROLL"
for f in $GEN/codegen/Source/*.c "$@" $(find $TE/src/kernels/int_forward_op $TE/src/kernels/fp_requantize_op $CM/NN/Source -name '*.c'); do
  gcc $F $INC -c "$f" -o $B/$(basename ${f%.c})_$(echo $f | md5sum | cut -c1-6).o; done
gcc $F $INC "$H" $B/*.o -o "$OUT" -lm -Wl,--gc-sections -Wl,--allow-multiple-definition
rm -rf $B
