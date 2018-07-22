#include "solver/support/action_executor.h"

#include <utility>

ActionExecutor::ActionExecutor(std::unique_ptr<Action> action)
    : tick_executor_(this), action_(std::move(action)) {
}

void ActionExecutor::Run(FieldState* field, TraceWriter* writer) {
  while (!done_) {
    tick_executor_.Run(field, writer);
  }
}

void ActionExecutor::Decide(TickExecutor::Commander* commander) {
  done_ = action_->Decide(commander);
}
