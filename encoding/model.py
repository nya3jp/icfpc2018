import gzip
import math
import numpy as np


class Model:
    """Init with model file file path and accessible like numpy array
    Model.decode_model(path) method returns bool matrix representing model
    Model.encode_model(path, data) method dumps matrix to .mdl file

    """

    def __init__(self, path):
        self.matrix = Model.decode_model(path)
        self.resolution = self.matrix.shape[0]

    def __getitem__(self, item):
        return self.matrix[item]

    @classmethod
    def decode_model(cls, path, from_gzip=False):
        if from_gzip:
            opener = gzip.open
        else:
            opener = open
        with opener(path, 'rb') as bf:
            bf.seek(0)
            data = np.frombuffer(bf.read(), dtype=np.uint8)
        resolution = data[0]
        ret = np.empty((resolution,) * 3, dtype=bool)
        R = resolution
        for x in range(R):
            for y in range(R):
                for z in range(R):
                    i = x * R ** 2 + y * R + z + 8
                    ret[x, y, z] = bool(data[i // 8] & (1 << (i % 8)))
        return ret

    @classmethod
    def encode_model(cls, path, data):
        size = int(math.ceil((data.shape[0] ** 3 + 1) / 8) + .1)
        ret = np.zeros(size, dtype=np.uint8)
        resolution = data.shape[0]
        ret[0] = resolution
        for i in range(data.shape[0] ** 3):
            x = i // resolution ** 2
            y = (i % resolution ** 2) // resolution
            z = i % resolution
            ret[i // 8 + 1] += (1 << (i % 8)) if data[x, y, z] else 0
        encoded = ret.tobytes()
        with open(path, 'wb') as bf:
            bf.write(encoded)
