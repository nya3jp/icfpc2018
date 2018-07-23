#ifndef SOLVER_DATA_STATE_H
#define SOLVER_DATA_STATE_H

#include <stdint.h>

#include <map>
#include <string>
#include <utility>

#include "solver/data/geometry.h"
#include "solver/data/matrix.h"

class BotState {
 public:
  BotState(int id, uint64_t seeds, Point position);

  BotState(const BotState& other) = delete;
  BotState(BotState&& other) {
    *this = std::move(other);
  }
  BotState& operator=(const BotState& other) = delete;
  BotState& operator=(BotState&& other) {
    if (&other != this) {
      id_ = other.id_;
      seeds_ = other.seeds_;
      position_ = std::move(other.position_);
    }
    return *this;
  }

  int id() const { return id_; }
  uint64_t seeds() const { return seeds_; }
  const Point& position() const { return position_; }

  void set_id(int id) { id_ = id; }
  void set_seeds(uint64_t seeds) { seeds_ = seeds; }
  void set_position(const Point& position) { position_ = position; }

  std::string ToJSON() const;

 private:
  int id_;
  uint64_t seeds_;
  Point position_;
};

class FieldState {
 public:
  FieldState(Matrix target, Matrix matrix, std::map<int, BotState> bots)
      : energy_(0),
        target_(std::move(target)),
        matrix_(std::move(matrix)),
        bots_(std::move(bots)),
        harmonics_(Harmonics::LOW) {}

  FieldState(const FieldState& other) = delete;
  FieldState(FieldState&& other) {
    *this = std::move(other);
  }
  FieldState& operator=(const FieldState& other) = delete;
  FieldState& operator=(FieldState&& other) {
    if (&other != this) {
      matrix_ = std::move(other.matrix_);
      bots_ = std::move(other.bots_);
    }
    return *this;
  }

  static FieldState FromModels(Matrix source, Matrix target);

  const Matrix& target() const { return target_; }

  Matrix& matrix() { return matrix_; }
  const Matrix& matrix() const { return matrix_; }

  std::map<int, BotState>& bots() { return bots_; }
  const std::map<int, BotState>& bots() const { return bots_; }

  bool IsHalted() const { return bots_.empty(); }

  uint64_t energy() const { return energy_; }
  void IncrementEnergy(int diff) { energy_ += diff; }

  bool IsHarmonicsLow() const { return harmonics_ == Harmonics::LOW; }
  void FlipHarmonics() { harmonics_ = harmonics_ == Harmonics::LOW ? Harmonics::HIGH : Harmonics::LOW; }

  std::string ToJSON() const;

 private:
  uint64_t energy_;
  enum class Harmonics { LOW, HIGH };
  const Matrix target_;
  Matrix matrix_;
  std::map<int, BotState> bots_;
  Harmonics harmonics_;
};

#endif // SOLVER_DATA_STATE_H
