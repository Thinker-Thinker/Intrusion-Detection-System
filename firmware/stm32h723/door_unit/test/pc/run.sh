#!/usr/bin/env bash
# PC test of the door logic (door.c + admin.c with fake hardware). Usage: test/pc/run.sh
set -e; cd "$(dirname "$0")"; D=../..
gcc -O1 -DPCSIM -Wall -Wno-unused-function -Wno-format-truncation -Wno-stringop-truncation -Istub -I$D/Core/Inc \
  $D/Core/Src/door.c $D/Core/Src/admin.c fakes.c door_logic_test.c -lm -o /tmp/door_logic_test
/tmp/door_logic_test
