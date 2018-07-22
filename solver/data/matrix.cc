#include "solver/data/matrix.h"

// static
Matrix Matrix::FromResolution(int resolution) {
  return Matrix(resolution, std::vector<bool>(resolution * resolution * resolution, false));
}

bool Matrix::IsEmpty() const {
  for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
    if (data_[i]) {
      return false;
    }
  }
  return true;
}

bool Matrix::Get(int x, int y, int z) const {
  CHECK(0 <= x && x < resolution_ &&
        0 <= y && y < resolution_ &&
        0 <= z && z < resolution_)
      << "(" << x << ", " << y << ", " << z << "); resolution=" << resolution_;
  return data_[x * resolution_ * resolution_ + y * resolution_ + z];
}

void Matrix::Set(int x, int y, int z, bool value) {
  CHECK(0 <= x && x < resolution_ &&
        0 <= y && y < resolution_ &&
        0 <= z && z < resolution_)
      << "(" << x << ", " << y << ", " << z << "); resolution=" << resolution_;
  data_[x * resolution_ * resolution_ + y * resolution_ + z] = value;
}

Matrix Matrix::Copy() const {
  return Matrix(resolution_, data_);
}
