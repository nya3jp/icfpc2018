import argparse
from encoding.model import Model
from encoding.trace import Trace


class SlightlyBetterSolution:
    def __init__(self, model_path):
        self.model = Model(model_path)
        self.command = list()
        self.trace = Trace()
        self.position = (0, 0, 0)
        self.filled = self.model.matrix.copy()
        self.filled.fill(False)

    def _is_same_direction(self, a, b):
        if a[1] == 0 and a[2] == 0 and b[1] == 0 and b[2] == 0:
            return True
        if a[0] == 0 and a[1] == 0 and b[0] == 0 and b[1] == 0:
            return True
        if a[0] == 0 and a[2] == 0 and b[0] == 0 and b[2] == 0:
            return True
        return False

    def _is_sld(self, a, b):
        if abs(a[0]) > 5 or abs(a[1]) > 5 or abs(a[2]) > 5:
            return False
        if abs(b[0]) > 5 or abs(b[1]) > 5 or abs(b[2]) > 5:
            return False
        dx, dy, dz = a[0] + b[0], a[1] + b[1], a[2] + b[2]
        if abs(dx) and abs(dy) and abs(dz):
            return False
        if abs(dx) <= 5 and abs(dy) <= 5 and abs(dz) <= 5:
            return True

    def _is_one_hot(self, a):
        return ((a[0], a[1]) == (0, 0)) or ((a[1], a[2]) == (0, 0)) or (
                (a[0], a[2]) == (0, 0))

    def _compile(self):
        # for i in self.command:
        #     self.trace.s_move(i)
        # self.command.clear()
        # return
        i = 0
        dx, dy, dz = 0, 0, 0
        first = None
        x, y, z = self.position
        while i < len(self.command):
            while (i < len(self.command) and
                   (self._is_same_direction((dx, dy, dz), self.command[i]) or
                    self._is_sld((dx, dy, dz), self.command[i]))):
                # todo これを外すと結果が向上するが壊れうる
                # if not self._is_same_direction((dx, dy, dz), self.command[i]):
                #     if (dx + self.command[i][0]) * dx < 0:
                #         self.command[i] = (self.command[i][0] + dx, 0, 0)
                #         dx = 0
                #     elif (dy + self.command[i][1]) * dy < 0:
                #         self.command[i] = (0, self.command[i][0] + dy, 0)
                #         dy = 0
                #     elif (dz + self.command[i][2]) * dz < 0:
                #         self.command[i] = (0, 0, self.command[i][0] + dz)
                #         dz = 0
                #     if self.command[i] == (0, 0, 0):
                #         i += 1
                #     break
                dx += self.command[i][0]
                dy += self.command[i][1]
                dz += self.command[i][2]
                if self._is_one_hot((dx, dy, dz)):
                    first = (dx, dy, dz)
                i += 1
            if self._is_one_hot((dx, dy, dz)):
                while abs(dx) + abs(dy) + abs(dz) > 15:
                    if abs(dx) > 0:
                        self.trace.s_move((15 if dx > 0 else -15, 0, 0))
                        dx -= 15 if dx > 0 else -15
                    if abs(dy) > 0:
                        self.trace.s_move((0, 15 if dy > 0 else -15, 0))
                        dy -= 15 if dy > 0 else -15
                    if abs(dz) > 0:
                        self.trace.s_move((0, 0, 15 if dz > 0 else -15))
                        dz -= 15 if dz > 0 else -15
                else:
                    if abs(dx) + abs(dy) + abs(dz):
                        self.trace.s_move((dx, dy, dz))
                        dx, dy, dz = 0, 0, 0
            else:
                if first[0]:
                    self.trace.l_move((dx, 0, 0), (0, dy, dz))
                if first[1]:
                    self.trace.l_move((0, dy, 0), (dx, 0, dz))
                if first[2]:
                    self.trace.l_move((0, 0, dz), (dx, dy, 0))
                first = None
                dx, dy, dz = 0, 0, 0
        self.command.clear()
        self.position = x, y, z
        return

    def simulate(self):
        model = self.model
        trace = self.trace
        command = self.command
        matrix = model.matrix
        resolution = self.model.resolution
        trace.flip()
        x, y, z = 0, 0, 0

        while y < resolution:
            # xy平面で端っこの場所を列挙
            # [x_b, x_t] x [z_b, z_t] にしかblockが存在しない
            x_t, x_b, z_t, z_b = None, None, None, None
            for i in range(1, resolution):
                if x_t is None and not matrix[i:, y, :].any():
                    x_t = i - 1
                if z_t is None and not matrix[:, y, i:].any():
                    z_t = i - 1
            for i in range(resolution, 0, -1):
                if x_b is None and not matrix[:i, y, :].any():
                    x_b = i
                if z_b is None and not matrix[:, y, :i].any():
                    z_b = i
            # とりあえず一番近い端から行くことにする
            sx, tx = (x_b, x_t) if abs(x - x_b) < abs(x - x_t) else (x_t, x_b)
            sz, tz = (z_b, z_t) if abs(z - z_b) < abs(z - z_t) else (z_t, z_b)
            if sx != x:
                command.append((sx - x, 0, 0))
                x = sx
            if sz != z:
                command.append((0, 0, sz - z))
                z = sz
            # memo: 本当はもう一つ内側からstartしても良い

            # naiveに進む
            dx = 1 if tx > sx else -1
            dz = 1 if tz > sz else -1

            # memo: 全てのサイドが空いていることを仮定している
            while x != tx + dx:
                while z != tz:
                    # if (0 <= x - dx < resolution and matrix[x - dx, y, z]
                    #         and not self.filled[x - dx, y, z]):
                    #     self._compile()
                    #     trace.fill((-dx, 0, 0))
                    #     self.filled[x - dx, y, z] = True
                    # if (0 <= z - dz < resolution and matrix[x, y, z - dz]
                    #         and not self.filled[x, y, z - dz]):
                    #     self._compile()
                    #     trace.fill((0, 0, -dz))
                    #     self.filled[x, y, z - dz] = True
                    command.append((0, 0, dz))
                    if matrix[x, y, z]:
                        self._compile()
                        trace.fill((0, 0, -dz))
                        self.filled[x, y, z] = True
                    z += dz
                dz *= -1
                sz, tz = tz, sz
                # if (0 <= x - dx < resolution and matrix[x - dx, y, z]
                #         and not self.filled[x - dx, y, z]):
                #     self._compile()
                #     trace.fill((-dx, 0, 0))
                #     self.filled[x - dx, y, z] = True
                # if (0 <= z - dz < resolution and matrix[x, y, z - dz]
                #         and not self.filled[x, y, z - dz]):
                #     self._compile()
                #     trace.fill((0, 0, -dz))
                #     self.filled[x, y, z - dz] = True
                if x != tx:
                    command.append((dx, 0, 0))
                    if matrix[x, y, z]:
                        self._compile()
                        trace.fill((-dx, 0, 0))
                        self.filled[x, y, z] = True
                    x += dx
                else:
                    break

            if not y == resolution - 1:
                command.append((0, 1, 0))
                y += 1
                if matrix[x, y - 1, z]:
                    self._compile()
                    trace.fill((0, -1, 0))
                    self.filled[x, y - 1, z] = True
            if not matrix[:, y:, :].any():
                break

        # back to home
        while x > 0:
            command.append((-1, 0, 0))
            x -= 1
        while z > 0:
            command.append((0, 0, -1))
            z -= 1
        while y > 0:
            command.append((0, -1, 0))
            y -= 1
        self._compile()
        trace.flip()
        trace.halt()

    def encode(self, path):
        self.trace.encode(path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('model_path', type=str)
    parser.add_argument('output_path', type=str)
    args = parser.parse_args()

    solution = SlightlyBetterSolution(
        args.model_path,
    )
    solution.simulate()
    solution.encode(args.output_path)
    trace = Trace()
    trace.decode(args.output_path)
    # trace.debug_print()
