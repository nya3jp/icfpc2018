#ifndef SOLVER_SUPPORT_ACTION_EXECUTOR_H
#define SOLVER_SUPPORT_ACTION_EXECUTOR_H

#include <memory>

#include "solver/data/state.h"
#include "solver/io/trace_writer.h"
#include "solver/support/actions/base.h"
#include "solver/support/tick_executor.h"

class ActionExecutor : public TickExecutor::Strategy {
 public:
  explicit ActionExecutor(std::unique_ptr<Action> action);
  ActionExecutor(const ActionExecutor& other) = delete;

  void Run(FieldState* field, TraceWriter* writer);

  // Implements TickExecutor::Strategy.
  void Decide(TickExecutor::Commander* commander) override;

 private:
  TickExecutor tick_executor_;
  std::unique_ptr<Action> action_;
  bool done_ = false;
};

#endif //SOLVER_SUPPORT_ACTION_EXECUTOR_H
