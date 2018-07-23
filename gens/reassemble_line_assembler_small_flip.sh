#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for n in `seq -f "%03g" 1 34`; do
  for z in 1 2; do
    ./bazel-bin/solver/solver \
      --source data/models/FR${n}_src.mdl \
      --target data/models/FR${n}_tgt.mdl \
      --output a.nbt \
      --line_assembler_flip_xz \
      --line_assembler_x_divs 1 \
      --line_assembler_z_divs ${z} \
      --impl reassemble_naive \
      --disasm bbgvoid_task \
      --asm line_assembler
    ./evaluate.py R $n a.nbt --nobuild
  done
done

