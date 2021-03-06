import numpy as np
from typing import List, Tuple


class CoordDifference():

    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def mlen(self):
        return abs(self.x) + abs(self.y) + abs(self.z)

    def clen(self):
        return max(abs(self.x), abs(self.y), abs(self.z))

    def is_linear(self):
        return int(self.x != 0) + int(self.y != 0) + int(self.z != 0) == 1

    def is_sld(self):
        return self.is_linear() and max(self.x, self.y, self.z) <= 5

    def is_lld(self):
        return self.is_linear() and max(self.x, self.y, self.z) <= 15

    def is_nd(self):
        return 0 < self.mlen() <= 2 and self.clen(d) == 1


class Bot():

    def __init__(self, bid: int, pos: Tuple[int, int, int], seeds: List[int]):
        assert bid > 0, bid
        self.bid = bid
        self.pos = pos
        self.seeds = seeds

    @staticmethod
    def get_initial_bot():
        return Bot(1, (0, 0, 0), list(range(2, 21)))


class Command():

    def serialize():
        raise NotImplementedError


class Halt(Command):

    def __init__(self):
        pass


class Wait(Command):

    def __init__(self):
        pass


class Flip(Command):

    def __init__(self):
        pass


class SMove(Command):

    def __init__(self, lld):
        self.lld = lld


class LMove(Command):

    def __init__(self, sld1, sld2):
        self.sld1 = sld1
        self.sld2 = sld2


class Fission(Command):

    def __init__(self, nd, m):
        self.nd = nd
        self.m = m


class Fill(Command):

    def __init__(self, nd):
        self.nd = nd


class FusionP(Command):

    def __init__(self, nd):
        self.nd = nd


class FusionS(Command):

    def __init__(self, nd):
        self.nd = nd


class Error(Exception):
    pass


class CommandError(Error):
    pass


class State():

    def __init__(self, resolution: int):
        assert 0 < resolution <= 250, resolution
        self.resolution = resolution
        self.energy = 0
        self.is_high = False
        shape = resolution, resolution, resolution
        # Values in matrix is 0: Void, 1: Full.
        self.matrix = np.zeros(shape, dtype=np.int)
        self.bots = [Bot.get_initial_bot()]
        self.trace = []

    def is_well_formed(self):
        if self.is_high:
            p_grounded = True
        else:
            p_grounded = is_grounded(self.matrix)
        bids = [b.bid for b in self.bots]
        poss = [b.pos for b in self.bots]
        p_different_bids = len(bids) == len(set(bids))
        p_different_poss = len(poss) == len(set(poss))

        p_poss_void = True
        for x, y, z in poss:
            p_poss_void = p_poss_void and self.matrix[x, y, z] != 0

        seed_sets = [set(b.seeds) for b in self.bots]
        set_acc = set()
        len_acc = 0
        for s in seed_sets:
            set_acc |= s
            len_acc += len(s)
        p_disjoint_seeds = len(set_acc) == len_acc
        p_no_bid_in_seeds = all([i not in set_acc for i in bids])

    def find_bot(self, bid):
        for b in self.bots:
            if b.bid == bid:
                return b
        assert False, bid

    def get_volatile_coordinates(self, bid: int, command: Command):
        bot = self.find_bot(bid)
        if isinstance(command, (Halt, Wait, Flip)):
            return [bot.pos]
        else:
            raise NotImplementedError

    def set_trace(self, trace: List[Command]):
        raise NotImplementedError

    def execute_step(self):
        raise NotImplementedError


def is_grounded(matrix):
    grounded = np.zeros_like(matrix, dtype=np.bool)
    nx, ny, nz = matrix.shape

    def neighbor_full_voxels(x, y, z):
        neighbors = [(x - 1, y, z),
                     (x + 1, y, z),
                     (x, y - 1, z),
                     (x, y + 1, z),
                     (x, y, z - 1),
                     (x, y, z + 1)]

        def valid(x, y, z):
            return (0 <= x < nx and
                    0 <= y < ny and
                    0 <= z < nz and
                    matrix[x, y, z] != 0 and
                    not grounded[x, y, z])

        return [n for n in neighbors if valid(*n)]

    # Init: y = 0 => grounded.
    xs, zs = np.where(matrix[:, 0, :] != 0)
    grounded[xs, 0, zs] = True
    last_grounded = []
    for x, z in zip(xs, zs):
        last_grounded.append((x, 0, z))
    # BFS.
    while last_grounded != []:
        last_grounded_next = []
        for x, y, z in last_grounded:
            ns = neighbor_full_voxels(x, y, z)
            for n in ns:
                grounded[n] = True
                last_grounded_next.append(n)
        last_grounded = last_grounded_next
    mxs, mys, mzs = np.where(matrix != 0)
    gxs, gys, gzs = np.where(grounded)
    return np.all(mxs == gxs) and np.all(mys == gys) and np.all(mzs == gzs)


def main():
    state = State(10)


if __name__ == '__main__':
    main()
