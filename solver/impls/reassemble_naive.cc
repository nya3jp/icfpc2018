#include "solver/impls/reassemble_naive.h"

#include "glog/logging.h"

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/impls/bbgvoid_task.h"
#include "solver/impls/deleter.h"
#include "solver/impls/fission_naive.h"
#include "solver/impls/generic_task.h"
#include "solver/impls/naive.h"
#include "solver/io/trace_writer.h"
#include "solver/tasks/line_assembler.h"

ReassembleNaiveSolver::ReassembleNaiveSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer,
    const std::string& disassembler_name, const std::string& assembler_name)
    : source_(source), target_(target), writer_(writer),
      disassembler_name_(disassembler_name), assembler_name_(assembler_name) {
}

void ReassembleNaiveSolver::Solve() {
  Matrix empty = Matrix::FromResolution(source_->Resolution());

  std::unique_ptr<Solver> disassembler, assembler;
  if (disassembler_name_ == "bbgvoid_task") {
    disassembler = std::unique_ptr<Solver>(new BBGvoidTaskSolver(source_, &empty, writer_, false));
  } else if (disassembler_name_ == "delete") {
    disassembler = std::unique_ptr<Solver>(new DeleteStrategySolver(source_, &empty, writer_, false));
  } else {
    LOG(FATAL) << "No disassembler impl found.";
  }
  if (assembler_name_ == "naive") {
    assembler = std::unique_ptr<Solver>(new NaiveSolver(&empty, target_, writer_));
  } else if (assembler_name_ == "fission_naive") {
    assembler = std::unique_ptr<Solver>(new FissionNaiveSolver(&empty, target_, writer_));
  } else if (assembler_name_ == "line_assembler") {
    assembler = std::unique_ptr<Solver>(new GenericTaskSolver(MakeLineAssemblerTask(), &empty, target_, writer_));
  } else {
    LOG(FATAL) << "No assembler impl found.";
  }
  disassembler->Solve();
  assembler->Solve();
}
