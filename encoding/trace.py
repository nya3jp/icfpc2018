import argparse
import pathlib
import sys
import numpy as np


class Trace:
    """ How to decode:
    trace = Trace()
    trace.decode('path')
    trace.debug_print()

    How to encode:
    trace = Trace()
    trace.wait()
    trace.flip()
    trace.s_move((0, 1, 0))
    trace.flip()
    trace.halt()
    trace.encode('path')

    """

    def __init__(self):
        self.commands = list()
        self.idx = 0

    def clear(self):
        self.commands = list()
        self.idx = 0

    def next(self):
        assert self.idx <= len(self.commands)
        if self.idx == len(self.commands):
            return [0b11111110]
        if self.commands[self.idx] in [0b11111111,
                                       0b11111110,
                                       0b11111101]:
            ret = [self.commands[self.idx]]
            self.idx += 1
            return ret
        if self.commands[self.idx] & 0b111 in [0b111,
                                               0b110,
                                               0b011]:
            ret = [self.commands[self.idx]]
            self.idx += 1
            return ret
        if self.commands[self.idx] & 0b111 in [0b001,
                                               0b000]:
            ret = [self.commands[self.idx + x] for x in range(4)]
            self.idx += 4
            return ret
        ret = [self.commands[self.idx], self.commands[self.idx + 1]]
        self.idx += 2
        return ret

    def halt(self):
        self.commands.append(0b11111111)

    def wait(self):
        self.commands.append(0b11111110)

    def flip(self):
        self.commands.append(0b11111101)

    def s_move(self, lld):
        lld_a, lld_i = Trace._encode_lld(lld)
        self.commands.append(0b00000100 + (lld_a << 4))
        self.commands.append(lld_i)

    def l_move(self, sld1, sld2):
        sld1_a, sld1_i = Trace._encode_sld(sld1)
        sld2_a, sld2_i = Trace._encode_sld(sld2)
        self.commands.append((sld2_a << 6) + (sld1_a << 4) + 0b1100)
        self.commands.append((sld2_i << 4) + sld1_i)

    def fusion_p(self, nd):
        nd = Trace._encode_nd(nd)
        self.commands.append((nd << 3) + 0b111)

    def fusion_s(self, nd):
        nd = Trace._encode_nd(nd)
        self.commands.append((nd << 3) + 0b110)

    def fission(self, nd, m):
        nd = Trace._encode_nd(nd)
        self.commands.append((nd << 3) + 0b101)
        self.commands.append(m)

    def fill(self, nd):
        nd = Trace._encode_nd(nd)
        self.commands.append((nd << 3) + 0b011)

    def void(self, nd):
        nd = Trace._encode_nd(nd)
        self.commands.append((nd << 3) + 0b010)

    def gfill(self, nd, fd):
        nd = Trace._encode_nd(nd)
        fd = Trace._encode_fd(fd)
        self.commands.append((nd << 3) + 0b001)
        self.commands.append(fd[0])
        self.commands.append(fd[1])
        self.commands.append(fd[2])

    def gvoid(self, nd, fd):
        nd = Trace._encode_nd(nd)
        fd = Trace._encode_fd(fd)
        self.commands.append((nd << 3) + 0b000)
        self.commands.append(fd[0])
        self.commands.append(fd[1])
        self.commands.append(fd[2])

    def encode(self, path):
        data = np.asarray(self.commands, dtype=np.uint8).tobytes()
        with open(path, 'wb') as wf:
            wf.write(data)

    def decode(self, path):
        with open(path, 'rb') as f:
            self.commands = np.frombuffer(f.read(), dtype=np.uint8)

    def debug_print(self, file=sys.stdout):
        data = self.commands
        skip = False
        for i in range(len(data)):
            # print('{0:08b}'.format(data[i]))
            if skip:
                skip = False
                continue
            command = data[i] & 0b111
            if data[i] == 0b11111111:  # halt
                print('Halt', file=file)
            elif data[i] == 0b11111110:  # wait
                print('Wait', file=file)
            elif data[i] == 0b11111101:  # flip
                print('Flip', file=file)
            elif command == 0b100:  # move
                skip = True
                if data[i] & 0b1000:  # l_move
                    axis2 = (data[i] & 0b11000000) >> 6
                    dist2 = (data[i + 1] & 0b11110000) >> 4
                    axis1 = (data[i] & 0b00110000) >> 4
                    dist1 = data[i + 1] & 0b00001111
                    print('LMove',
                          Trace._decode_sld(axis1, dist1),
                          Trace._decode_sld(axis2, dist2), file=file)
                else:  # s_move
                    axis = (data[i] & 0b00110000) >> 4
                    dist = data[i + 1] & 0b11111
                    print('SMove', Trace._decode_lld(axis, dist), file=file)
            elif command == 0b111:  # fusion_p
                nd = (data[i] & 0b11111000) >> 3
                print('FusionP', Trace._decode_nd(nd), file=file)
            elif command == 0b110:  # fusion_s
                nd = (data[i] & 0b11111000) >> 3
                print('FusionS', Trace._decode_nd(nd), file=file)
            elif command == 0b101:  # fission
                skip = True
                nd = (data[i] & 0b11111000) >> 3
                print('Fission', Trace._decode_nd(nd), data[i + 1], file=file)
            elif command == 0b011:  # fill
                nd = (data[i] & 0b11111000) >> 3
                print('Fill', Trace._decode_nd(nd), file=file)
            elif command == 0b010:  # void
                nd = (data[i] & 0b11111000) >> 3
                print('Void', Trace._decode_nd(nd), file=file)
            elif command == 0b001:  # gfill
                # TODO: skip
                nd = (data[i] & 0b11111000) >> 3
                fd = data[i + 1:i + 4]
                print('GFill', Trace._decode_nd(nd), Trace._decode_fd(fd), file=file)
            elif command == 0b000:  # gvoid
                # TODO: skip
                nd = (data[i] & 0b11111000) >> 3
                fd = data[i + 1:i + 4]
                print('GVoid', Trace._decode_nd(nd), Trace._decode_fd(fd), file=file)
            else:
                raise ValueError('unrecognized command {0:08b}'.format(data[i]))

    @staticmethod
    def _encode_ld(ld):
        if ld[0] != 0:
            assert (ld[1], ld[2]) == (0, 0), 'ld is not one-hot-vector'
            return 1, ld[0]
        elif ld[1] != 0:
            assert (ld[0], ld[2]) == (0, 0), 'ld is not one-hot-vector'
            return 2, ld[1]
        elif ld[2] != 0:
            assert (ld[0], ld[1]) == (0, 0), 'ld is not one-hot-vector'
            return 3, ld[2]
        else:
            raise ValueError('ld must be non-zero')

    @staticmethod
    def _decode_ld(axis, dist):
        if axis == 1:
            return dist, 0, 0
        if axis == 2:
            return 0, dist, 0
        if axis == 3:
            return 0, 0, dist
        raise ValueError('invalid axis: {0}'.format(axis))

    @staticmethod
    def _encode_sld(sld):
        encoded, dist = Trace._encode_ld(sld)
        assert -5 <= dist <= 5, 'sld must satisfy (manhattan(sld) <= 5)'
        return encoded, dist + 5

    @staticmethod
    def _decode_sld(axis, dist):
        dist -= 5
        assert -5 <= dist <= 5, 'sld must satisfy (manhattan(sld) <= 5)'
        return Trace._decode_ld(axis, dist)

    @staticmethod
    def _encode_lld(lld):
        encoded, dist = Trace._encode_ld(lld)
        assert -15 <= dist <= 15, 'lld must satisfy (manhattan(sld) <= 15)'
        return encoded, dist + 15

    @staticmethod
    def _decode_lld(axis, dist):
        dist -= 15
        assert -15 <= dist <= 15, 'lld must satisfy (manhattan(lld) <= 15)'
        return Trace._decode_ld(axis, dist)

    @staticmethod
    def _encode_nd(nd):
        for coord in nd:
            assert -1 <= coord <= 1, 'each element of nd must be smaller than 2'
        assert sum(abs(i) for i in nd) <= 2, \
            'nd must satisfy manhattan(nd) <= 2'
        return (nd[0] + 1) * 9 + (nd[1] + 1) * 3 + (nd[2] + 1)

    @staticmethod
    def _decode_nd(nd):
        decoded_nd = nd // 9 - 1, (nd % 9) // 3 - 1, nd % 3 - 1
        for coord in decoded_nd:
            assert -1 <= coord <= 1, 'invalid nd: {0}'.format(nd)
        return decoded_nd

    @staticmethod
    def _encode_fd(fd):
        return fd[0] + 30, fd[1] + 30, fd[2] + 30

    @staticmethod
    def _decode_fd(fd):
        return fd[0] - 30, fd[1] - 30, fd[2] - 30


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('root_dir', type=str, help='path to the dfltTracesL')
    args = parser.parse_args()

    root_dir = pathlib.Path(args.root_dir)
    trace = Trace()
    for path in root_dir.glob('*.nbt'):
        path = str(path)
        trace.clear()
        trace.decode(path)
        with open(path[:-4] + '.txt', 'w') as wf:
            trace.debug_print(wf)
