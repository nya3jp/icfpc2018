#include "solver/impls/action_executor_example.h"

#include <utility>

#include "glog/logging.h"

#include "solver/support/action_executor.h"
#include "solver/support/actions/mock.h"

ActionExecutorExampleSolver::ActionExecutorExampleSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
}

void ActionExecutorExampleSolver::Solve() {
  std::unique_ptr<Action> action(
      new MockAction(Action::MakeBotSet(field_.bots())));
  ActionExecutor executor(std::move(action));
  executor.Run(&field_, writer_);
  CHECK(field_.IsHalted()) << "Not halted, but expected";
}
