#ifndef SOLVER_IMPLS_TICK_EXECUTOR_EXAMPLE_H
#define SOLVER_IMPLS_TICK_EXECUTOR_EXAMPLE_H

#include "solver/data/matrix.h"
#include "solver/data/state.h"
#include "solver/impls/base.h"
#include "solver/io/trace_writer.h"

class TickExecutorExampleSolver : public Solver {
 public:
  TickExecutorExampleSolver(const Matrix* source, const Matrix* target, TraceWriter* writer);
  TickExecutorExampleSolver(const TickExecutorExampleSolver& other) = delete;

  void Solve() override;

 private:
  FieldState field_;
  TraceWriter* writer_;
};

#endif //SOLVER_IMPLS_TICK_EXECUTOR_EXAMPLE_H
