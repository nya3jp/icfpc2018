#ifndef SOLVER_SUPPORT_ACTIONS_BASE_H
#define SOLVER_SUPPORT_ACTIONS_BASE_H

#include <map>

#include "solver/data/state.h"
#include "solver/support/tick_executor.h"

// Represents an action of one or more bots.
class Action {
 public:
  using BotSet = std::map<int, const BotState*>;
  using Commander = TickExecutor::Commander;

  explicit Action(const BotSet& bots) : bots_(bots) {}
  virtual ~Action() = default;

  // Decides the commands to perform for this tick.
  // This function should return true if the action is finished by this call of
  // Decide() or it has already finished.
  virtual bool Decide(Commander* commander) = 0;

  // Returns the set of bots assigned to this action.
  const BotSet& bots() const { return bots_; }

  // Utility function to make BotSet.
  static BotSet MakeBotSet(const std::map<int, BotState>& bots);

 private:
  const BotSet bots_;
};

#endif //SOLVER_SUPPORT_ACTIONS_BASE_H
