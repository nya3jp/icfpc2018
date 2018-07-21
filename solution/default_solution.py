import argparse
from encoding.model import Model
from encoding.trace import Trace


class DefaultSolution:
    def __init__(self, model_path, shortcut=False):
        self.model = Model(model_path)
        self.trace = Trace()
        self.shortcut = shortcut

    def simulate(self):
        model = self.model
        trace = self.trace
        matrix = model.matrix
        resolution = self.model.resolution
        trace.flip()
        x_dir, y_dir, z_dir = 1, 1, 1
        x, y, z = 0, 0, 0
        if self.shortcut:
            trace.s_move((0, 1, 0))
            y += 1
        while y < resolution:
            if x_dir > 0:
                while y > 0 and model[:x, y - 1, :].any():
                    trace.s_move((-1, 0, 0))
                    x -= 1
            else:
                while y > 0 and model[x + 1:, y - 1, :].any():
                    trace.s_move((1, 0, 0))
                    x += 1
            while (x_dir > 0 and x < resolution - 1) or (
                    x_dir < 0 and x > 0):
                if z_dir > 0:
                    while y > 0 and model[x, y - 1, :z].any():
                        trace.s_move((0, 0, -1))
                        z -= 1
                else:
                    while y > 0 and model[x, y - 1, z + 1:].any():
                        trace.s_move((0, 0, 1))
                        z += 1
                while (z_dir > 0 and z < resolution - 1) or (
                        z_dir < 0 and z > 0):
                    if y > 0 and model[x, y - 1, z]:
                        trace.fill((0, -1, 0))
                    trace.s_move((0, 0, z_dir))
                    z += z_dir
                    if self.shortcut:
                        if z_dir > 0 and not matrix[x, y - 1, z:].any():
                            break
                        elif z_dir < 0 and not matrix[x, y - 1, :z + 1].any():
                            break
                z_dir *= -1
                if y > 0 and model[x, y - 1, z]:
                    trace.fill((0, -1, 0))
                trace.s_move((x_dir, 0, 0))
                x += x_dir
                if self.shortcut:
                    if x_dir > 0 and not matrix[x:, y - 1, :].any():
                        break
                    elif x_dir < 0 and not matrix[:x + 1, y - 1, :].any():
                        break
            x_dir *= -1
            if y > 0 and model[x, y - 1, z]:
                trace.fill((0, -1, 0))
            if self.shortcut and not matrix[:, y:, :].any():
                break
            if y + 1 == resolution:
                break
            trace.s_move((0, y_dir, 0))
            y += y_dir
        while x > 0:
            trace.s_move((-1, 0, 0))
            x -= 1
        while z > 0:
            trace.s_move((0, 0, -1))
            z -= 1
        while y > 0:
            trace.s_move((0, -1, 0))
            y -= 1
        trace.flip()
        trace.halt()

    def encode(self, path):
        self.trace.encode(path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('model_path', type=str)
    parser.add_argument('output_path', type=str)
    parser.add_argument('--shortcut', action='store_true')
    args = parser.parse_args()

    dfltsltn = DefaultSolution(
        args.model_path,
        shortcut=args.shortcut,
    )
    dfltsltn.simulate()
    dfltsltn.encode(args.output_path)
