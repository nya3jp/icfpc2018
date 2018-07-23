#include "solver/impls/generic_task.h"

#include <utility>

#include "glog/logging.h"

#include "solver/support/task_executor.h"

GenericTaskSolver::GenericTaskSolver(
    TaskPtr task, const Matrix* source, const Matrix* target, TraceWriter* writer)
    : task_(std::move(task)),
      field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
}

void GenericTaskSolver::Solve() {
  TaskExecutor executor(std::move(task_));
  executor.Run(&field_, writer_);
  if (!field_.IsHalted()) {
    LOG(ERROR) << "Trace not halted.";
  }
}
