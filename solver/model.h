#ifndef SOLVER_MODEL_H
#define SOLVER_MODEL_H

#include <iostream>
#include <memory>
#include <vector>

class Model {
 public:
  Model(const Model& other) = delete;

  static std::unique_ptr<Model> FromStream(std::istream& is);

  bool Get(int x, int y, int z) const;

  int Resolution() const {
    return n_;
  }

 private:
  Model(int n, std::vector<bool> data) : n_(n), data_(data) {}

  int n_;
  std::vector<bool> data_;
};

#endif // SOLVER_MODEL_H
