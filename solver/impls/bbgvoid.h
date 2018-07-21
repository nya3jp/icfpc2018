#ifndef SOLVER_IMPLS_BBGVOID_H
#define SOLVER_IMPLS_BBGVOID_H

#include "solver/data/model.h"
#include "solver/impls/base.h"
#include "solver/support/careless_controller.h"

class BBGvoidSolver : public Solver {
 public:
  BBGvoidSolver(const Model* model, TraceWriter* writer)
      : model_(model), controller_(model->Resolution(), writer) {}
  BBGvoidSolver(const BBGvoidSolver& other) = delete;

  virtual void Solve();

 private:
  const Model* const model_;
  CarelessController controller_;
};

#endif // SOLVER_IMPLS_BBGVOID_H
