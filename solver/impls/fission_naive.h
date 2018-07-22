#ifndef SOLVER_IMPLS_FISSION_NAIVE_H
#define SOLVER_IMPLS_FISSION_NAIVE_H

#include "solver/data/matrix.h"
#include "solver/data/state.h"
#include "solver/impls/base.h"
#include "solver/io/trace_writer.h"

class FissionNaiveSolver : public Solver {
 public:
  FissionNaiveSolver(const Matrix* source, const Matrix* target, TraceWriter* writer);
  FissionNaiveSolver(const FissionNaiveSolver& other) = delete;

  void Solve() override;

 private:
  FieldState field_;
  TraceWriter* writer_;
};

#endif //SOLVER_IMPLS_FISSION_NAIVE_H
