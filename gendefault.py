#!/usr/bin/env python3

import json
import os
import sys

import evaluate


ROOT_DIR = os.path.dirname(__file__)


def generate(taskname, src, tgt):
    src_model_path = os.path.join(ROOT_DIR, 'data', 'models', taskname + '_src.mdl.gz') if src else None
    tgt_model_path = os.path.join(ROOT_DIR, 'data', 'models', taskname + '_tgt.mdl.gz') if tgt else None
    trace_path = os.path.join(ROOT_DIR, 'data', 'defaults', taskname + '.nbt.gz')
    default_meta_path = os.path.join(ROOT_DIR, 'data', 'defaults', taskname + '.json')
    time, energy = evaluate._run_reference_simulator(src_model_path, tgt_model_path, trace_path)
    meta = {'time': time, 'energy': energy}
    with open(default_meta_path, 'w') as f:
        json.dump(meta, f)
    print('Saved {}'.format(default_meta_path))


def main():
    tasks = {'A': 186, 'D': 186, 'R': 115}
    for kind in ['A', 'D', 'R']:
        src, tgt = {'A': (False, True), 'D': (True, False), 'R': (True, True)}[kind]
        for model_id in range(1, tasks[kind] + 1):
            taskname = 'F{}{:03d}'.format(kind, model_id)
            generate(taskname, src, tgt)


if __name__ == '__main__':
    main()
