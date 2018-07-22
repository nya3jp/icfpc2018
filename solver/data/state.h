#ifndef SOLVER_DATA_STATE_H
#define SOLVER_DATA_STATE_H

#include <stdint.h>

#include <map>
#include <utility>

#include "solver/data/geometry.h"
#include "solver/data/matrix.h"

class BotState {
 public:
  BotState(int id, uint64_t seeds, Point position);
  BotState(const BotState& other) = delete;

  int id() const { return id_; }
  uint64_t seeds() const { return seeds_; }
  const Point& position() const { return position_; }

  void set_seeds(uint64_t seeds) { seeds_ = seeds; }
  void set_position(const Point& position) { position_ = position; }

 private:
  const int id_;
  uint64_t seeds_;
  Point position_;
};

class FieldState {
 public:
  FieldState(Matrix matrix, std::map<int, BotState> bots)
      : matrix_(std::move(matrix)), bots_(std::move(bots)) {}
  FieldState(const FieldState& other) = delete;

  Matrix& matrix() { return matrix_; }
  const Matrix& matrix() const { return matrix_; }

  std::map<int, BotState>& bots() { return bots_; }
  const std::map<int, BotState>& bots() const { return bots_; }

  bool IsHalted() const { return bots_.empty(); }

 private:
  Matrix matrix_;
  std::map<int, BotState> bots_;
};

#endif // SOLVER_DATA_STATE_H
