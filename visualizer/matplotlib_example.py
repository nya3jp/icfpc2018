#%matplotlib notebook
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

import encoding.model as model

model_path = '../data/models/LA001_tgt.mdl.gz'
m = model.Model.decode_model(model_path, from_gzip=True)
resolution = m.shape[0]
x, y, z = np.where(m)

fig = plt.figure()
ax = Axes3D(fig)
ax.plot(x, y, z, 's', alpha=0.8, markersize=10)
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_xticks(range(resolution))
ax.set_yticks(range(resolution))
ax.set_zticks(range(resolution))
ax.set_xlim(0, resolution)
ax.set_ylim(0, resolution)
ax.set_zlim(0, resolution)

plt.show()
