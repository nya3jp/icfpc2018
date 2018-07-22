#ifndef SOLVER_IMPLS_BBGVOID_H
#define SOLVER_IMPLS_BBGVOID_H

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/support/careless_controller.h"

class BBGvoidSolver : public Solver {
 public:
  BBGvoidSolver(const Matrix* source, const Matrix* target, TraceWriter* writer)
      : model_(source), controller_(source->Resolution(), writer) {
    CHECK(target->IsEmpty()) << "Can solve disassembly problem only";
  }
  BBGvoidSolver(const BBGvoidSolver& other) = delete;

  virtual void Solve();

 private:
  const Matrix* const model_;
  CarelessController controller_;
};

#endif // SOLVER_IMPLS_BBGVOID_H
