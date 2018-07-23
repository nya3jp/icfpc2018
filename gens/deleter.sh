#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for i in $(seq -f %03g 1 186); do
  bazel-bin/solver/solver \
      --impl=delete3 \
      --source=data/models/FD${i}_src.mdl \
      --output=a.nbt && \
      ./evaluate.py D $i a.nbt --nobuild
done
