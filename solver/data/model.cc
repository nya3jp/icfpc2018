#include "solver/data/model.h"

#include <stdint.h>

bool Model::Get(int x, int y, int z) const {
  CHECK(0 <= x && x < resolution_ &&
        0 <= y && y < resolution_ &&
        0 <= z && z < resolution_)
      << "(" << x << ", " << y << ", " << z << "); resolution=" << resolution_;
  return data_[x * resolution_ * resolution_ + y * resolution_ + z];
}
