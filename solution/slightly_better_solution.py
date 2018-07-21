import argparse
from encoding.model import Model
from encoding.trace import Trace


class SlightlyBetterSolution:
    def __init__(self, model_path):
        self.model = Model(model_path)
        self.command = list()
        self.trace = Trace()
        self.position = (0, 0, 0)

    def _compile(self):
        # ここで自分の位置をkeepした方がいい
        x, y, z = self.position
        for dx, dy, dz in self.command:
            x += dx
            y += dy
            z += dz
        self.command.clear()
        self.position = x, y, z

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
                if x_t is not None and not matrix[i:, y, :].any():
                    x_t = i - 1
                if z_t is not None and not matrix[:, y, i:].any():
                    z_t = i - 1
            for i in range(resolution, 0, -1):
                if x_b is not None and not matrix[:i, y, :].any():
                    x_b = i
                if z_b is not None and not matrix[:, y, :i].any():
                    z_b = i
            # とりあえず一番近い端から行くことにする
            sx, tx = (x_b, x_t) if abs(x - x_b) < abs(x - x_t) else (x_t, x_b)
            sz, tz = (z_b, z_t) if abs(z - z_b) < abs(z - z_t) else (z_t, z_b)
            command.append((sx - x, 0, sz - z))
            # memo: 本当はもう一つ内側からstartしても良い

            # naiveに進む
            dx = 1 if tx > sx else -1
            dz = 1 if z > tz else -1
            while x != tx + dx:
                while z != tz + dz:
                    command.append((0, 0, dz))
                    if matrix[x, y, z]:
                        self._compile()
                        trace.fill((0, 0, -dz))
                    z += dz
                dz *= -1
                sz, tz = tz, sz
                command.append((dx, 0, 0))
                if matrix[x, y, z]:
                    self._compile()
                    trace.fill((-dx, 0, 0))
                x += dx

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