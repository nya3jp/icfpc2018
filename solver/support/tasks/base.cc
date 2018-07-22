#include "solver/support/tasks/base.h"

// static
Task::BotSet Task::MakeBotSet(const std::map<int, BotState>& bots) {
  BotSet botset;
  for (const auto& pair : bots) {
    botset.emplace(pair.first, &pair.second);
  }
  return botset;
}
