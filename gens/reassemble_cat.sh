#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

# 14-11: 7
./bazel-bin/solver/solver \
  --source problemsF/FD014_src.mdl \
  --output a.nbt \
  --impl bbgvoid_task \
  --nohalt

cat handasm/FA011.nbt >>a.nbt
./evaluate.py R 7 a.nbt --nobuild

# 11-12: 4
./bazel-bin/solver/solver \
  --source problemsF/FD011_src.mdl \
  --output a.nbt \
  --impl bbgvoid_task \
  --nohalt

gunzip -dc data/traces/FA012.nbt.gz >>a.nbt
./evaluate.py R 4 a.nbt --nobuild

# 12-13: 5
./bazel-bin/solver/solver \
  --source problemsF/FD012_src.mdl \
  --output a.nbt \
  --impl bbgvoid_task \
  --nohalt

gunzip -dc data/traces/FA013.nbt.gz >>a.nbt
./evaluate.py R 5 a.nbt --nobuild

# 18-15: 11
./bazel-bin/solver/solver \
  --source problemsF/FD018_src.mdl \
  --output a.nbt \
  --impl bbgvoid_task \
  --nohalt

gunzip -dc data/traces/FA015.nbt.gz >>a.nbt
./evaluate.py R 11 a.nbt --nobuild

