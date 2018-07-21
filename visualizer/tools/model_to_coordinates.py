import sys

import numpy as np

import encoding.model as model

model_path = sys.argv[1]
out_path = sys.argv[2]
m = model.Model.decode_model(model_path, from_gzip=True)
resolution = m.shape[0]
xs, ys, zs = np.where(m)

acc = []
with open(out_path, 'w') as fp:
    print('coordinates = [', file=fp)
    for x, y, z in zip(xs, ys, zs):
        xc, yc, zc = (p * 100 for p in (x, y, z))
        print('    [{:d}, {:d}, {:d}],'.format(xc, yc, zc), file=fp)
    print('];', file=fp)
