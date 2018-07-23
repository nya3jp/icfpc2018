#!/bin/bash

cd "$(dirname "$0")/.."

bazel build ...

for x in `seq 1 8`; do
    for z in `seq 1 8`; do
        if [ `expr $x \* $z` -gt 20 ]; then
            continue
        fi

        # No flip.
        for i in $(seq -f %03g 1 30); do
            bazel-bin/solver/solver \
                --line_assembler_x_divs $x \
                --line_assembler_z_divs $z \
                --impl line_assembler \
                --target data/models/FA${i}_tgt.mdl \
                --output a.nbt
            ./evaluate.py A $i a.nbt --nobuild
        done

        # Flip.
        for i in $(seq -f %03g 1 30); do
            bazel-bin/solver/solver \
                --line_assembler_flip_xz \
                --line_assembler_x_divs $x \
                --line_assembler_z_divs $z \
                --impl line_assembler \
                --target data/models/FA${i}_tgt.mdl \
                --output a.nbt
            ./evaluate.py A $i a.nbt --nobuild
        done
    done
done
