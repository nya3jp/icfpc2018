#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "solver/data/model.h"
#include "solver/impls/base.h"
#include "solver/impls/naive.h"
#include "solver/impls/bbgvoid.h"
#include "solver/io/model_reader.h"
#include "solver/io/trace_writer.h"

DEFINE_string(model, "", "Path to input model file");
DEFINE_string(trace, "", "Path to output trace file");
DEFINE_string(impl, "", "Solver implementation name");

std::unique_ptr<Solver> CreateSolver(
    const std::string& name, const Model* model, TraceWriter* writer) {
  if (name == "naive") {
    return std::unique_ptr<Solver>(new NaiveSolver(model, writer));
  }
  if (name == "bbgvoid") {
    return std::unique_ptr<Solver>(new BBGvoidSolver(model, writer));
  }
  LOG(FATAL) << "No solver impl found. Set --impl correctly.";
  return nullptr;
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  if (FLAGS_model.empty() || FLAGS_trace.empty()) {
    LOG(ERROR) << "Usage: " << argv[0] << " --model=LA000.mdl --trace=out.nbt";
    return 1;
  }

  Model model;
  {
    std::ifstream fin(FLAGS_model.c_str());
    model = ReadModel(fin);
  }

  std::ofstream fout(FLAGS_trace.c_str());
  TraceWriter writer(fout);

  auto solver = CreateSolver(FLAGS_impl, &model, &writer);
  solver->Solve();

  return 0;
}
