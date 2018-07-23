//
// Created by Yusuke Tsuzuku on 2018/07/23.
//

#ifndef ICFPC2018_FA_H
#define ICFPC2018_FA_H

#include <set>

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/support/tick_executor.h"

class FA : public TickExecutor::Strategy {
 public:
  FA(const Matrix *target, bool halt, int num)
      : model_(target), area_(0), height_(-1), state_(State::FISSION),
        start_x_(-1), end_x_(-1), start_z_(-1), end_z_(-1), dz_(1), finished_(false), halt_(halt), num_(num) {
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
  int num_;
};

class FASolver : public Solver {
 public:
  FASolver(const Matrix *source, const Matrix *target, TraceWriter *writer, bool halt = true, int num = 0);
//  DeleteStrategySolver(const DeleteStrategySolver& other) = delete;

  void Solve() override;

 private:
  const Matrix *const model_;
  bool halt_;
  FieldState field_;
  TraceWriter *writer_;
  int num_;
};

#endif //ICFPC2018_FA_H
