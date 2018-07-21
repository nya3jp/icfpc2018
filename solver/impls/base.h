#ifndef SOLVER_IMPLS_BASE_H
#define SOLVER_IMPLS_BASE_H

class Solver {
 public:
  virtual ~Solver() = default;
  Solver(const Solver& other) = delete;

  virtual void Solve() = 0;

 protected:
  Solver() = default;
};

#endif //SOLVER_IMPLS_BASE_H
