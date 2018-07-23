#ifndef SOLVER_IMPLS_GENERIC_TASK_H
#define SOLVER_IMPLS_GENERIC_TASK_H

#include "solver/data/matrix.h"
#include "solver/data/state.h"
#include "solver/impls/base.h"
#include "solver/io/trace_writer.h"
#include "solver/support/task.h"

class GenericTaskSolver : public Solver {
 public:
  GenericTaskSolver(TaskPtr task, const Matrix* source, const Matrix* target, TraceWriter* writer);
  GenericTaskSolver(const GenericTaskSolver& other) = delete;

  void Solve() override;

 private:
  TaskPtr task_;
  FieldState field_;
  TraceWriter* writer_;
};

#endif //SOLVER_IMPLS_GENERIC_TASK_H
