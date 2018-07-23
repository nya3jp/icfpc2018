#ifndef SOLVER_IMPLS_REASSEMBLE_NAIVE_H
#define SOLVER_IMPLS_REASSEMBLE_NAIVE_H

#include <string>

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/io/trace_writer.h"

class ReassembleNaiveSolver : public Solver {
 public:
  ReassembleNaiveSolver(
      const Matrix* source, const Matrix* target, TraceWriter* writer,
      const std::string& disassembler_name, const std::string& assembler_name);
  ReassembleNaiveSolver(const ReassembleNaiveSolver& other) = delete;

  void Solve() override;

 private:
  const Matrix* source_;
  const Matrix* target_;
  TraceWriter* writer_;
  const std::string disassembler_name_;
  const std::string assembler_name_;
};

#endif //SOLVER_IMPLS_REASSEMBLE_NAIVE_H
