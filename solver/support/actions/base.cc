#include "solver/support/actions/base.h"

// static
Action::BotSet Action::MakeBotSet(const std::map<int, BotState>& bots) {
  BotSet botset;
  for (const auto& pair : bots) {
    botset.emplace(pair.first, &pair.second);
  }
  return botset;
}
