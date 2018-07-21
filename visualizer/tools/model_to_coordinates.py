import sys

import numpy as np

import encoding.model as model

model_path = sys.argv[1]
out_path = sys.argv[2]
m = model.Model.decode_model(model_path, from_gzip=True)
resolution = m.shape[0]
xs, ys, zs = np.where(m)
mesh_size = 100

acc = []
with open(out_path, 'w') as fp:
    # Draw boxes.
    print('coordinates_box = [', file=fp)
    for x, y, z in zip(xs, ys, zs):
        xc, yc, zc = (p * mesh_size + mesh_size // 2 for p in (x, y, z))
        r, g, b = (int(p / resolution * 256) for p in (x, y, z))
        color = r * 0x10000 + g * 0x100 + b
        print('    [{:d}, {:d}, {:d}, {:d}],'.format(xc, yc, zc, color), file=fp)
    print('];', file=fp)
    print(file=fp)
    # Draw grid.
    print('coordinates_line = [', file=fp)
    template = '    [[{:d}, {:d}, {:d}], [{:d}, {:d}, {:d}]],'
    x_start = 0
    x_end = resolution * mesh_size
    for yi in range(resolution):
        for zi in range(resolution):
            y = yi * mesh_size
            z = zi * mesh_size
            print(template.format(x_start, y, z, x_end, y, z), file=fp)
    y_start = 0
    y_end = resolution * mesh_size
    for xi in range(resolution):
        for zi in range(resolution):
            x = xi * mesh_size
            z = zi * mesh_size
            print(template.format(x, y_start, z, x, y_end, z), file=fp)
    z_start = 0
    z_end = resolution * mesh_size
    for xi in range(resolution):
        for yi in range(resolution):
            x = xi * mesh_size
            y = yi * mesh_size
            print(template.format(x, y, z_start, x, y, z_end), file=fp)
    print('];', file=fp)
