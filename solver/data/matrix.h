#ifndef SOLVER_DATA_MATRIX_H
#define SOLVER_DATA_MATRIX_H

#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "solver/data/geometry.h"

class Matrix {
 public:
  Matrix() : resolution_(0) {}
  Matrix(int resolution, std::vector<bool> data)
      : resolution_(resolution), data_(std::move(data)) {
    CHECK_EQ(static_cast<int>(data_.size()), resolution * resolution * resolution);
  }

  // Matrix is movable but not copyable by default. Use explicit Copy() instead.
  Matrix(const Matrix& other) = delete;
  Matrix(Matrix&& other) {
    *this = std::move(other);
  }
  Matrix& operator=(const Matrix& other) = delete;
  Matrix& operator=(Matrix&& other) {
    if (&other != this) {
      resolution_ = other.resolution_;
      data_ = std::move(other.data_);
    }
    return *this;
  }

  bool operator==(const Matrix& other) const;
  bool operator!=(const Matrix& other) const;

  static Matrix FromResolution(int resolution);

  bool IsZeroSized() const { return resolution_ == 0; }
  bool IsEmpty() const;
  bool IsMovable(const Region& r) const;
  bool IsPlaceable(const Region& r) const;
  bool Get(int x, int y, int z) const;
  int Resolution() const { return resolution_; }

  void Set(int x, int y, int z, bool value);

  Matrix Copy() const;

  std::string ToJSON();

 private:
  int resolution_;
  std::vector<bool> data_;
};

#endif // SOLVER_DATA_MATRIX_H
