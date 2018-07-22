#include "solver/impls/tick_executor_example.h"

#include "solver/support/tick_executor.h"

namespace {

class Strategy : public TickExecutor::Strategy {
 public:
  Strategy() = default;
  Strategy(const Strategy& other) = delete;

  void Decide(TickExecutor::Commander* commander) override;
};

void Strategy::Decide(TickExecutor::Commander* commander) {
  for (const auto& pair : commander->commands()) {
    int bot_id = pair.first;
    commander->Set(bot_id, Command::Halt());
  }
}

}  // namespace

TickExecutorExampleSolver::TickExecutorExampleSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
}

void TickExecutorExampleSolver::Solve() {
  Strategy strategy;
  TickExecutor executor(&strategy);
  while (!field_.IsHalted()) {
    executor.Run(&field_, writer_);
  }
}
