#ifndef ICFPC2018_DELETER_H
#define ICFPC2018_DELETER_H

#include <set>

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/support/tick_executor.h"

class DeleterStrategy : public TickExecutor::Strategy {
 public:
  DeleterStrategy(const Matrix *source)
      : model_(source), area_(0), height_(-1), state_(State::FISSION) {
  }
//  DeleterStrategy(const DeleterStrategy &other) = delete;

  void Decide(TickExecutor::Commander *commander);
  enum State {
    FISSION,
    DELETION,
    ELEVATE,
    SLIDE,
    FUSION,
    RETURN,
  };

 private:
  const Matrix *const model_;
  State state_;
  int area_;
  int height_;
};

class DeleteStrategySolver : public Solver {
 public:
  DeleteStrategySolver(const Matrix* source, const Matrix* target, TraceWriter* writer);
//  DeleteStrategySolver(const DeleteStrategySolver& other) = delete;

  void Solve() override;

 private:
  const Matrix *const model_;
  FieldState field_;
  TraceWriter* writer_;
};

#endif //ICFPC2018_DELETER_H
