#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for n in `seq -f "%03g" 1 34`; do
  ./bazel-bin/solver/solver \
    --source data/models/FR${n}_src.mdl \
    --target data/models/FR${n}_tgt.mdl \
    --output a.nbt \
    --impl reassemble_naive \
    --disasm bbgvoid_task \
    --asm line_assembler
  ./evaluate.py R $n a.nbt --nobuild
done

for n in `seq -f "%03g" 35 115`; do
  ./bazel-bin/solver/solver \
    --source data/models/FR${n}_src.mdl \
    --target data/models/FR${n}_tgt.mdl \
    --output a.nbt \
    --impl reassemble_naive \
    --disasm delete3 \
    --asm line_assembler
  ./evaluate.py R $n a.nbt --nobuild
done
