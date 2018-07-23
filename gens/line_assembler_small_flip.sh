#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for i in $(seq -f %03g 1 30); do
  bazel-bin/solver/solver \
      --line_assembler_flip_xz \
      --line_assembler_x_divs 1 \
      --line_assembler_z_divs 1 \
      --impl line_assembler \
      --target data/models/FA${i}_tgt.mdl \
      --output a.nbt
  ./evaluate.py A $i a.nbt --nobuild
done

for i in $(seq -f %03g 1 30); do
  bazel-bin/solver/solver \
      --line_assembler_flip_xz \
      --line_assembler_x_divs 2 \
      --line_assembler_z_divs 1 \
      --impl line_assembler \
      --target data/models/FA${i}_tgt.mdl \
      --output a.nbt
  ./evaluate.py A $i a.nbt --nobuild
done
