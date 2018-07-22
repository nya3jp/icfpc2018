#include "solver/support/task_executor.h"

#include <utility>

TaskExecutor::TaskExecutor(std::unique_ptr<Task> task)
    : tick_executor_(this), task_(std::move(task)) {
}

void TaskExecutor::Run(FieldState* field, TraceWriter* writer) {
  while (!done_) {
    tick_executor_.Run(field, writer);
  }
}

void TaskExecutor::Decide(TickExecutor::Commander* commander) {
  done_ = task_->Decide(commander);
}
