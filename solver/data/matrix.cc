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

bool Matrix::IsInSpace(const Region& r) const {
  return (0 <= r.mini.x && r.maxi.x < resolution_ &&
          0 <= r.mini.y && r.maxi.y < resolution_ &&
          0 <= r.mini.z && r.maxi.z < resolution_);
}

bool Matrix::IsMovable(const Region& r) const {
  if (!IsInSpace(r)) {
    return false;
  }
  for (int x = r.mini.x; x <= r.maxi.x; ++x) {
    for (int y = r.mini.y; y <= r.maxi.y; ++y) {
      for (int z = r.mini.z; z <= r.maxi.z; ++z) {
        if (data_[x * resolution_ * resolution_ + y * resolution_ + z]) {
          return false;
        }
      }
    }
  }
  return true;
}

bool Matrix::IsPlaceable(const Region& r) const {
  CHECK(1 <= r.mini.x && r.maxi.x < resolution_ - 1 &&
        0 <= r.mini.y && r.maxi.y < resolution_ - 1 &&
        1 <= r.mini.z && r.maxi.z < resolution_ - 1);
  return true;
}

bool Matrix::Contains(const Point& p) const {
  return (0 <= p.x && p.x < resolution_ &&
          0 <= p.y && p.y < resolution_ &&
          0 <= p.z && p.z < resolution_);
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

std::string Matrix::ToJSON() const {
  std::string s = "[";
  std::string sep = ",";
  int last = data_[0];
  int last_count = 0;
  for (int i = 0; i < static_cast<int>(data_.size()); ++i) { // TODO: auto
    int current = data_[i];
    if (current == last) {
      last_count++;
    } else {
      s += std::to_string(last) + sep + std::to_string(last_count) + sep;
      last = current;
      last_count = 1;
    }
  }
  if (last_count != 0) {
    s += std::to_string(last) + sep + std::to_string(last_count) + sep;
  }
  s.pop_back();
  s += "]";
  return s;
}

bool Matrix::operator==(const Matrix& other) const {
  return resolution_ == other.resolution_ && data_ == other.data_;
}

bool Matrix::operator!=(const Matrix& other) const {
  return !(*this == other);
}
