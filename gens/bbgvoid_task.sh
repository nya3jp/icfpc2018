#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for n in `seq -f "%03g" 1 41`; do
  ./bazel-bin/solver/solver \
    --source data/models/FD${n}_src.mdl \
    --output a.nbt \
    --impl bbgvoid_task
  ./evaluate.py D $n a.nbt --nobuild
done
