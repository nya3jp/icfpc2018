#ifndef ICFPC2018_DELETER3_H
#define ICFPC2018_DELETER3_H

#include <set>

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/support/tick_executor.h"

class DeleterStrategy3 : public TickExecutor::Strategy {
 public:
  DeleterStrategy3(const Matrix *source, bool halt)
      : model_(source), area_(0), height_(-1), state_(State::FISSION),
        start_x_(-1), end_x_(-1), start_z_(-1), end_z_(-1), dz_(1), finished_(false), halt_(halt) {
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
  int start_x_;
  int end_x_;
  int start_z_;
  int end_z_;
  int dz_;
  bool halt_;
  bool finished_;
};

class DeleteStrategySolver3 : public Solver {
 public:
  DeleteStrategySolver3(const Matrix *source, const Matrix *target, TraceWriter *writer, bool halt = true);
//  DeleteStrategySolver(const DeleteStrategySolver& other) = delete;

  void Solve() override;

 private:
  const Matrix *const model_;
  bool halt_;
  FieldState field_;
  TraceWriter *writer_;
};

#endif //ICFPC2018_DELETER3_H
