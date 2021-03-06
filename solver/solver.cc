#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "solver/data/matrix.h"
#include "solver/impls/base.h"
#include "solver/impls/bbgvoid.h"
#include "solver/impls/bbgvoid_task.h"
#include "solver/impls/deleter.h"
#include "solver/impls/deleter3.h"
#include "solver/impls/fa.h"
#include "solver/impls/fission_naive.h"
#include "solver/impls/generic_task.h"
#include "solver/impls/naive.h"
#include "solver/impls/reassemble_naive.h"
#include "solver/impls/task_executor_example.h"
#include "solver/impls/tick_executor_example.h"
#include "solver/io/model_reader.h"
#include "solver/io/trace_writer_impl.h"
#include "solver/manuals/index.h"
#include "solver/tasks/line_assembler.h"
#include "solver/tasks/manual_assembler.h"

DEFINE_string(source, "", "Path to source model file");
DEFINE_string(target, "", "Path to target model file");
DEFINE_string(output, "", "Path to output trace file");
DEFINE_string(impl, "", "Solver implementation name");
DEFINE_string(disasm, "", "Disassembler name used in ReassembleNaive");
DEFINE_string(asm, "", "Disassembler name used in ReassembleNaive");
DEFINE_bool(nohalt, false, "Do not output halt");

std::unique_ptr<Solver> CreateSolver(
    const std::string& name, const Matrix* source, const Matrix* target, TraceWriter* writer) {
  if (name == "naive") {
    return std::unique_ptr<Solver>(new NaiveSolver(source, target, writer));
  }
  if (name == "bbgvoid") {
    return std::unique_ptr<Solver>(new BBGvoidSolver(source, target, writer));
  }
  if (name == "bbgvoid_task") {
    return std::unique_ptr<Solver>(new BBGvoidTaskSolver(source, target, writer, !FLAGS_nohalt));
  }
  if (name == "fission_naive") {
    return std::unique_ptr<Solver>(new FissionNaiveSolver(source, target, writer));
  }
  if (name == "line_assembler") {
    return std::unique_ptr<Solver>(new GenericTaskSolver(MakeLineAssemblerTask(), source, target, writer));
  }
  if (name == "manual") {
    return std::unique_ptr<Solver>(new GenericTaskSolver(MakeManualAssemblerTask(MakeManualMainTask()), source, target, writer));
  }
  if (name == "tick_executor_example") {
    return std::unique_ptr<Solver>(new TickExecutorExampleSolver(source, target, writer));
  }
  if (name == "task_executor_example") {
    return std::unique_ptr<Solver>(new TaskExecutorExampleSolver(source, target, writer));
  }
  if (name == "delete") {
    return std::unique_ptr<Solver>(new DeleteStrategySolver(source, target, writer, !FLAGS_nohalt));
  }
  if (name == "delete3") {
    return std::unique_ptr<Solver>(new DeleteStrategySolver3(source, target, writer, !FLAGS_nohalt));
  }
  if (name == "fa015") {
    return std::unique_ptr<Solver>(new FASolver(source, target, writer, false, 15));
  }
  if (name == "reassemble_naive") {
    return std::unique_ptr<Solver>(new ReassembleNaiveSolver(source, target, writer, FLAGS_disasm, FLAGS_asm));
  }
  LOG(FATAL) << "No solver impl found. Set --impl correctly.";
  return nullptr;
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::InstallFailureSignalHandler();

  if ((FLAGS_source.empty() && FLAGS_target.empty()) || FLAGS_output.empty()) {
    LOG(ERROR) << "Usage: " << argv[0] << " [--source=LA000.mdl] [--target=LA000.mdl] --output=out.nbt";
    return 1;
  }

  Matrix source, target;
  if (!FLAGS_source.empty()) {
    source = ReadModel(FLAGS_source);
  }
  if (!FLAGS_target.empty()) {
    target = ReadModel(FLAGS_target);
  }

  if (source.IsZeroSized()) {
    source = Matrix::FromResolution(target.Resolution());
  }
  if (target.IsZeroSized()) {
    target = Matrix::FromResolution(source.Resolution());
  }

  CHECK_EQ(source.Resolution(), target.Resolution());

  std::ofstream fout(FLAGS_output.c_str());
  fout << std::unitbuf;
  TraceWriterImpl writer(fout);

  auto solver = CreateSolver(FLAGS_impl, &source, &target, &writer);
  solver->Solve();

  return 0;
}
