#ifndef SOLVER_IMPLS_NAIVE_H
#define SOLVER_IMPLS_NAIVE_H

#include "solver/data/model.h"
#include "solver/impls/base.h"
#include "solver/support/careless_controller.h"

class NaiveSolver : public Solver {
 public:
  NaiveSolver(const Model* model, TraceWriter* writer)
      : model_(model), controller_(model->Resolution(), writer) {}
  NaiveSolver(const NaiveSolver& other) = delete;

  virtual void Solve();

 private:
  const Point& Current() const;

  const Model* const model_;
  CarelessController controller_;
};

#endif // SOLVER_IMPLS_NAIVE_H
