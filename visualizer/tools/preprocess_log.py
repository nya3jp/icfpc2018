import json
import sys

import numpy as np


def read_matrix(num_voxels, matrix_log):
    matrix = np.zeros(num_voxels, dtype=np.int8)
    n = len(matrix_log)
    assert n % 2 == 0
    index = 0
    for i in range(0, n, 2):
        value = matrix_log[i]
        length = matrix_log[i + 1]
        matrix[index : index + length] = value
        index += length
    return matrix


def diff_to_json(diff, opacity):
    nonzero_indexes = np.where(diff != 0)
    nonzero_values = diff[nonzero_indexes]
    acc = []
    for i, v in zip(nonzero_indexes[0], nonzero_values):
        if v == 1:
            acc.append([int(i), True, 0x0000ff, opacity])
        elif v == -1:
            acc.append([int(i), False, 0, 0.0])
        else:
            assert False, v
    return acc


def main():
    log_path = sys.argv[1]
    out_path = sys.argv[2]
    with open(log_path) as fp:
        log = json.load(fp)
    num_voxels = sum(log['states'][0]['field']['matrix'][1::2])
    resolution = int(num_voxels ** (1.0 / 3.0) + 0.5)
    assert num_voxels == resolution ** 3

    with open(out_path, 'w') as fp:
        print('resolution = {:d}'.format(resolution), file=fp)

        def write_diff(matrix_before, matrix_after, transparent, tick, length):
            if transparent:
                opacity = 0.1
            else:
                opacity = 1.0
            diff = matrix_after - matrix_before
            diff_json = json.dumps(diff_to_json(diff, opacity))
            if tick < length - 1:
                sep = ','
            else:
                sep = ''
            print('  {}{}'.format(diff_json, sep), file=fp)

        print('target = ', file=fp)
        matrix_zero = np.zeros(num_voxels, dtype=np.int8)
        matrix = read_matrix(num_voxels, log['target'])
        write_diff(matrix_zero, matrix, True, 0, 1)
        print(file=fp)

        print('diffsForward = [', file=fp)
        matrix_last = np.zeros(num_voxels, dtype=np.int8)
        for tick, log_tick in enumerate(log['states']):
            matrix = read_matrix(num_voxels, log_tick['field']['matrix'])
            write_diff(matrix_last, matrix, False, tick, len(log['states']))
            matrix_last = matrix
        print(']\n', file=fp)

        # If there are 4 ticks 0, 1, 2, 3,
        # `diffsBackward` has 3 elements describing
        # diff from 1 to 0, 2 to 1 and 3 to 2.
        print('diffsBackward = [', file=fp)
        matrix_last = read_matrix(num_voxels, log['states'][0]['field']['matrix'])
        for tick, log_tick in enumerate(log['states'][1:]):
            matrix = read_matrix(num_voxels, log_tick['field']['matrix'])
            write_diff(matrix, matrix_last, False, tick, len(log['states']) - 1)
            matrix_last = matrix
        print(']\n', file=fp)

        print('botStates = [', file=fp)
        for tick, log_tick in enumerate(log['states']):
            bots_str = '  ['
            for i, bot in enumerate(log_tick['field']['bots']):
                pos = bot['bot_state']['position']
                bots_str += '[[{:d}, {:d}, {:d}], {:d}]'.format(*pos, 0xff0000)
                if i < len(log_tick['field']['bots']) - 1:
                    bots_str += ','
            bots_str += ']'
            if tick < len(log['states']) - 1:
                bots_str += ','
            print(bots_str, file=fp)
        print(']', file=fp)


if __name__ == '__main__':
    main()
