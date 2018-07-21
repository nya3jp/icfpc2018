import argparse
from encoding.model import Model
from encoding.trace import Trace


class DefaultSolution:
    def __init__(self, model_path):
        self.model = Model(model_path)
        self.trace = Trace()

    def simulate(self):
        model = self.model
        trace = self.trace
        resolution = self.model.resolution
        trace.flip()
        x_dir, y_dir, z_dir = 1, 1, 1
        place = [0, 0, 0]
        for y in range(resolution):
            for x in range(resolution - 1):
                for z in range(resolution - 1):
                    if place[1] > 0 and model[place[0], place[1] - 1, place[2]]:
                        trace.fill((0, -1, 0))
                    trace.s_move((0, 0, z_dir))
                    place[2] += z_dir
                z_dir *= -1
                if place[1] > 0 and model[place[0], place[1] - 1, place[2]]:
                    trace.fill((0, -1, 0))
                trace.s_move((x_dir, 0, 0))
                place[0] += x_dir
            x_dir *= -1
            if place[1] > 0 and model[place[0], place[1] - 1, place[2]]:
                trace.fill((0, -1, 0))
            if place[1] + 1 == resolution:
                break
            trace.s_move((0, y_dir, 0))
            place[1] += y_dir
        while place[0] > 0:
            trace.s_move((-1, 0, 0))
            place[0] -= 1
        while place[2] > 0:
            trace.s_move((0, 0, -1))
            place[2] -= 1
        while place[1] > 0:
            trace.s_move((0, -1, 0))
            place[1] -= 1
        trace.flip()
        trace.halt()

    def encode(self, path):
        self.trace.encode(path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('model_path', type=str)
    parser.add_argument('output_path', type=str)
    args = parser.parse_args()

    dfltsltn = DefaultSolution(args.model_path)
    dfltsltn.simulate()
    dfltsltn.encode(args.output_path)
