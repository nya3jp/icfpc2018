#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for i in $(seq -f %03g 1 186); do
  bazel-bin/solver/solver \
      --line_assembler_x_divs 5 \
      --line_assembler_z_divs 4 \
      --impl line_assembler \
      --target data/models/FA${i}_tgt.mdl \
      --output a.nbt
  ./evaluate.py A $i a.nbt --nobuild
done
