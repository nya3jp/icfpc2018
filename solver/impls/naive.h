#ifndef SOLVER_IMPLS_NAIVE_H
#define SOLVER_IMPLS_NAIVE_H

#include "glog/logging.h"

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/support/careless_controller.h"

class NaiveSolver : public Solver {
 public:
  NaiveSolver(const Matrix* source, const Matrix* target, TraceWriter* writer)
      : model_(target), controller_(target->Resolution(), writer) {
    CHECK(source->IsEmpty()) << "Can solve assembly problem only";
  }
  NaiveSolver(const NaiveSolver& other) = delete;

  void Solve() override;

 private:
  const Point& Current() const;

  const Matrix* const model_;
  CarelessController controller_;
};

#endif // SOLVER_IMPLS_NAIVE_H
