#!/usr/bin/env bash
# Build the firmware's face pipeline (door_unit) for the PC. Usage: build_pipeline.sh <door_unit dir> <out exe>
set -e
D=$(realpath "$1"); OUT=$(realpath -m "$2"); SIM=$(dirname "$(realpath "$0")"); TE=$D/TinyEngine
B=$(mktemp -d); mkdir -p $B/fakecore; printf '#include "arm_shim.h"\n' > $B/fakecore/cmsis_compiler.h; cp $B/fakecore/cmsis_compiler.h $B/fakecore/core_cm7.h
INC="-I$SIM -I$B/fakecore -I$D/Core/Inc -I$D/Models -I$TE/include -I$TE/CMSIS/NN/Include -I$TE/CMSIS/DSP/Include"
F="-O2 -w -DPCSIM -ffunction-sections -fdata-sections -include arm_shim.h -include te_protos.h -DARM_MATH_DSP -D__ARM_FEATURE_DSP=1 -DARM_MATH_LOOPUNROLL"
SRCS="$D/Models/*.c $D/Models/kernels/*.c $D/Core/Src/vision.c $D/Core/Src/face_models.c $(find $TE/src/kernels $TE/CMSIS/NN/Source -name '*.c')"
for f in $SRCS; do gcc $F $INC -c "$f" -o $B/$(basename ${f%.c})_$(echo $f | md5sum | cut -c1-6).o; done
gcc $F $INC $SIM/run_pipeline.c $B/*.o -o "$OUT" -lm -Wl,--gc-sections -Wl,--allow-multiple-definition
rm -rf $B
