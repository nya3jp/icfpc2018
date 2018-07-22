#ifndef SOLVER_IMPLS_TASK_EXECUTOR_EXAMPLE_H
#define SOLVER_IMPLS_TASK_EXECUTOR_EXAMPLE_H

#include "solver/data/matrix.h"
#include "solver/data/state.h"
#include "solver/impls/base.h"
#include "solver/io/trace_writer.h"

class TaskExecutorExampleSolver : public Solver {
 public:
  TaskExecutorExampleSolver(const Matrix* source, const Matrix* target, TraceWriter* writer);
  TaskExecutorExampleSolver(const TaskExecutorExampleSolver& other) = delete;

  void Solve() override;

 private:
  FieldState field_;
  TraceWriter* writer_;
};

#endif //SOLVER_IMPLS_TASK_EXECUTOR_EXAMPLE_H
