#include "solver/impls/task_executor_example.h"

#include <utility>

#include "glog/logging.h"

#include "solver/support/task_executor.h"
#include "solver/support/tasks/base.h"

TaskExecutorExampleSolver::TaskExecutorExampleSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
}

void TaskExecutorExampleSolver::Solve() {
  TaskExecutor executor(MakeTask(new MockTask()));
  executor.Run(&field_, writer_);
  CHECK(field_.IsHalted()) << "Not halted, but expected";
}
