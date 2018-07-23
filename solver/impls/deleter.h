#ifndef ICFPC2018_DELETER_H
#define ICFPC2018_DELETER_H

#include <set>

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/support/tick_executor.h"

class DeleterStrategy : public TickExecutor::Strategy {
 public:
  DeleterStrategy(const Matrix *source, bool halt)
      : model_(source), state_(State::FISSION), area_(0), height_(-1), finished_(false), halt_(halt) {
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
  bool Finished() { return finished_; }

 private:
  const Matrix *const model_;
  State state_;
  int area_;
  int height_;
  bool finished_;
  bool halt_;
};

class DeleteStrategySolver : public Solver {
 public:
  DeleteStrategySolver(const Matrix* source, const Matrix* target, TraceWriter* writer, bool halt = true);
//  DeleteStrategySolver(const DeleteStrategySolver& other) = delete;

  void Solve() override;

 private:
  FieldState field_;
  const Matrix *const model_;
  TraceWriter* writer_;
  bool halt_;
};

#endif //ICFPC2018_DELETER_H
