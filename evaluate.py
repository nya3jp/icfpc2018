#!/usr/bin/env python3

import json
import os
import subprocess
import sys


ROOT_DIR = os.path.dirname(__file__)


def _run_simulator(model_path, trace_path):
    simulator_path = os.path.join(ROOT_DIR, 'bazel-bin', 'cxx_simulator', 'sim')
    model_filter = '| gzip -d' if model_path.endswith('.gz') else ''
    trace_filter = '| gzip -d' if trace_path.endswith('.gz') else ''
    cmd = '"$0" <(cat "$1" %s) <(cat "$2" %s)' % (model_filter, trace_filter)
    output = subprocess.check_output(['bash', '-c', cmd, simulator_path, model_path, trace_path])
    time_str, energy_str = output.strip().split()
    return int(time_str), int(energy_str)


def main(argv):
    if len(argv) != 3:
        print('Usage: evaluate.py <model#> <trace-path>', file=sys.stderr)
        return 1

    model_id = int(argv[1])
    trace_path = argv[2]

    model_path = os.path.join(ROOT_DIR, 'data', 'models', 'LA%03d_tgt.mdl.gz' % model_id)
    best_trace_path = os.path.join(ROOT_DIR, 'data', 'traces', 'LA%03d.nbt.gz' % model_id)
    best_meta_path = os.path.join(ROOT_DIR, 'data', 'traces', 'LA%03d.json' % model_id)
    default_meta_path = os.path.join(ROOT_DIR, 'data', 'defaults', 'LA%03d.json' % model_id)

    with open(default_meta_path, 'r') as f:
        default_meta = json.load(f)

    default_energy = default_meta['energy']

    new_time, new_energy = _run_simulator(model_path, trace_path)
    new_meta = {
        'time': new_time,
        'energy': new_energy,
    }

    if os.path.exists(best_meta_path):
        with open(best_meta_path, 'r') as f:
            best_meta = json.load(f)
        best_energy = best_meta['energy']
    else:
        best_energy = default_energy

    print('-- Default:%12d' % default_energy)
    print('-- Best:   %12d' % best_energy)
    print('-- New:    %12d' % new_energy)

    if best_energy is not None and new_energy >= best_energy:
        print('Could not update the best record.')
        return 1

    print('Great, new record!')
    trace_filter = '' if trace_path.endswith('.gz') else '| gzip'
    subprocess.check_call(['bash', '-c', 'cat "$0" %s > "$1"' % trace_filter, trace_path, best_trace_path])
    with open(best_meta_path, 'w') as f:
        json.dump(new_meta, f, indent=2)
    print('Saved the following files:')
    print('  %s' % best_trace_path)
    print('  %s' % best_meta_path)
    print('Please commit them.')


if __name__ == '__main__':
    sys.exit(main(sys.argv))
