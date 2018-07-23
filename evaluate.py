#!/usr/bin/env python3

import json
import os
import subprocess
import sys


ROOT_DIR = os.path.dirname(__file__)


def _run_simulator(src_model_path, tgt_model_path, trace_path):
    simulator_path = os.path.join(ROOT_DIR, 'bazel-bin', 'cxx_simulator', 'sim')
    cmds = ['"$0"']
    args = []
    if src_model_path is not None:
        cmds.append('--source <(cat "${}" {})'.format(len(args) + 1, '| gzip -d' if src_model_path.endswith('.gz') else ''))
        args.append(src_model_path)
    if tgt_model_path is not None:
        cmds.append('--target <(cat "${}" {})'.format(len(args) + 1, '| gzip -d' if tgt_model_path.endswith('.gz') else ''))
        args.append(tgt_model_path)
    cmds.append('--trace <(cat "${}" {})'.format(len(args) + 1, '| gzip -d' if trace_path.endswith('.gz') else ''))
    args.append(trace_path)
    cmd = ' '.join(cmds)
    output = subprocess.check_output(['bash', '-c', cmd, simulator_path, *args])
    time_str, energy_str = output.strip().split()
    return int(time_str), int(energy_str)


def _run_reference_simulator(src_model_path, tgt_model_path, trace_path):
    simulator_path = os.path.join(ROOT_DIR, 'reference_simulator', 'simulator')
    src_model_path = src_model_path if src_model_path is not None else 'null'
    tgt_model_path = tgt_model_path if tgt_model_path is not None else 'null'
    cmd = ' '.join(['"$0"',
                    '<(cat "$1" | gzip -d)' if src_model_path.endswith('.gz') else '"$1"',
                    '<(cat "$2" | gzip -d)' if tgt_model_path.endswith('.gz') else '"$2"',
                    '<(cat "$3" | gzip -d)' if trace_path.endswith('.gz') else '"$3"'])
    output = subprocess.check_output(['bash', '-c', cmd, simulator_path, src_model_path, tgt_model_path, trace_path])
    time_str, energy_str = output.strip().split()
    return int(time_str), int(energy_str)


def main(argv):
    if len(argv) < 4 or argv[1] not in ('A', 'D', 'R'):
        print('Usage: evaluate.py <A|D|R> <model#> <trace-path>', file=sys.stderr)
        return 1

    use_reference = False
    skip_build = False
    for opt in argv[4:]:
        if opt == '--reference':
            use_reference = True
        elif opt == '--nobuild':
            skip_build = True
        else:
            print('Unknown option %s' % opt)
            return 1

    kind = argv[1]
    src, tgt = {'A': (False, True), 'D': (True, False), 'R': (True, True)}[kind]
    model_id = int(argv[2])
    trace_path = argv[3]

    taskname = 'F{}{:03d}'.format(kind, model_id)

    if not skip_build:
        subprocess.check_call(['bazel', 'build', '//cxx_simulator:sim'], stderr=subprocess.DEVNULL)

    src_model_path = os.path.join(ROOT_DIR, 'data', 'models', taskname + '_src.mdl.gz') if src else None
    tgt_model_path = os.path.join(ROOT_DIR, 'data', 'models', taskname + '_tgt.mdl.gz') if tgt else None
    best_trace_path = os.path.join(ROOT_DIR, 'data', 'traces', taskname + '.nbt.gz')
    best_meta_path = os.path.join(ROOT_DIR, 'data', 'traces', taskname + '.json')
    default_meta_path = os.path.join(ROOT_DIR, 'data', 'defaults', taskname + '.json')

    with open(default_meta_path, 'r') as f:
        default_meta = json.load(f)

    default_energy = default_meta['energy']

    if use_reference:
        new_time, new_energy = _run_reference_simulator(src_model_path, tgt_model_path, trace_path)
    else:
        new_time, new_energy = _run_simulator(src_model_path, tgt_model_path, trace_path)
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

    print('-- Default:%18d' % default_energy)
    print('-- Best:   %18d' % best_energy)
    print('-- New:    %18d' % new_energy)

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
