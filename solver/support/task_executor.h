#ifndef SOLVER_SUPPORT_TASK_EXECUTOR_H
#define SOLVER_SUPPORT_TASK_EXECUTOR_H

#include "solver/data/state.h"
#include "solver/io/trace_writer.h"
#include "solver/support/task.h"
#include "solver/support/tick_executor.h"

class TaskExecutor : public TickExecutor::Strategy {
 public:
  explicit TaskExecutor(TaskPtr task);
  TaskExecutor(const TaskExecutor& other) = delete;

  void Run(FieldState* field, TraceWriter* writer);

  // Implements TickExecutor::Strategy.
  void Decide(TickExecutor::Commander* commander) override;

 private:
  TickExecutor tick_executor_;
  TaskPtr task_;
  bool done_ = false;
};

#endif //SOLVER_SUPPORT_TASK_EXECUTOR_H
