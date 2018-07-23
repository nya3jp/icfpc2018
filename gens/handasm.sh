#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

cd handasm

PYTHONPATH=.. python3 FA006.py
PYTHONPATH=.. python3 FA011.py
PYTHONPATH=.. python3 FD003.py

../evaluate.py A 6 FA006.nbt --nobuild
../evaluate.py A 11 FA011.nbt --nobuild
../evaluate.py D 3 FD003.nbt --nobuild
