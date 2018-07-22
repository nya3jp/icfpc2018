#include "solver/support/tick_executor.h"

bool TickExecutor::Commander::Set(int bot_id, const Command& command) {
  // TODO(nya): Check feasibility of command!
  CHECK(0 <= bot_id && bot_id < static_cast<int>(commands_.size()));
  commands_[bot_id] = command;
  return true;
}

TickExecutor::Commander::Commander(const FieldState* field)
    : field_(field) {
  for (const auto& pair : field->bots()) {
    commands_[pair.first];  // Init with WAIT command.
  }
}

void TickExecutor::Run(FieldState* field, TraceWriter* writer) {
  Commander commander(field);
  strategy_->Decide(&commander);

  for (const auto& pair : commander.commands()) {
    ApplyCommand(pair.first, pair.second, field);
    writer->Command(pair.second);
  }
}

// static
void TickExecutor::ApplyCommand(int bot_id, const Command& command, FieldState* field) {
  LOG(FATAL) << "NOT IMPLEMENTED";
}
