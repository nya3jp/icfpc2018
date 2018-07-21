#ifndef SOLVER_DATA_MODEL_H
#define SOLVER_DATA_MODEL_H

#include <iostream>
#include <memory>
#include <vector>

class Model {
 public:
  static std::unique_ptr<Model> FromStream(std::istream& is);
  Model(const Model& other) = delete;

  bool Get(int x, int y, int z) const;
  int Resolution() const { return resolution_; }

 private:
  Model(int resolution, std::vector<bool> data)
      : resolution_(resolution), data_(std::move(data)) {}

  int resolution_;
  std::vector<bool> data_;
};

#endif // SOLVER_DATA_MODEL_H
