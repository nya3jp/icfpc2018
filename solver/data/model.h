#ifndef SOLVER_DATA_MODEL_H
#define SOLVER_DATA_MODEL_H

#include <utility>
#include <vector>

#include "glog/logging.h"

class Model {
 public:
  Model() : resolution_(0) {}
  Model(int resolution, std::vector<bool> data)
      : resolution_(resolution), data_(std::move(data)) {
    CHECK_EQ(static_cast<int>(data.size()), resolution * resolution * resolution);
  }

  Model(const Model& other) = delete;
  Model(Model&& other) {
    *this = std::move(other);
  }
  Model& operator=(const Model& other) = delete;
  Model& operator=(Model&& other) {
    if (&other != this) {
      resolution_ = other.resolution_;
      data_ = std::move(other.data_);
    }
    return *this;
  }

  bool Get(int x, int y, int z) const;
  int Resolution() const { return resolution_; }

 private:
  int resolution_;
  std::vector<bool> data_;
};

#endif // SOLVER_DATA_MODEL_H
