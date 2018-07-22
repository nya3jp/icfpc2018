#ifndef SOLVER_IMPLS_BBGVOID_TASK_H
#define SOLVER_IMPLS_BBGVOID_TASK_H

#include "solver/data/geometry.h"
#include "solver/data/matrix.h"
#include "solver/data/state.h"
#include "solver/impls/base.h"
#include "solver/io/trace_writer.h"

class BBGvoidTaskSolver : public Solver {
 public:
  BBGvoidTaskSolver(const Matrix* source, const Matrix* target, TraceWriter* writer);
  BBGvoidTaskSolver(const BBGvoidTaskSolver& other) = delete;

  void Solve() override;

 private:
  Region CalculateBB();

  FieldState field_;
  TraceWriter* writer_;
};

#endif //SOLVER_IMPLS_BBGVOID_TASK_H
