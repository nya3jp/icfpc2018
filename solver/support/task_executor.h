#ifndef SOLVER_SUPPORT_TASK_EXECUTOR_H
#define SOLVER_SUPPORT_TASK_EXECUTOR_H

#include <memory>

#include "solver/data/state.h"
#include "solver/io/trace_writer.h"
#include "solver/support/tasks/base.h"
#include "solver/support/tick_executor.h"

class TaskExecutor : public TickExecutor::Strategy {
 public:
  explicit TaskExecutor(std::unique_ptr<Task> task);
  TaskExecutor(const TaskExecutor& other) = delete;

  void Run(FieldState* field, TraceWriter* writer);

  // Implements TickExecutor::Strategy.
  void Decide(TickExecutor::Commander* commander) override;

 private:
  TickExecutor tick_executor_;
  std::unique_ptr<Task> task_;
  bool done_ = false;
};

#endif //SOLVER_SUPPORT_TASK_EXECUTOR_H
