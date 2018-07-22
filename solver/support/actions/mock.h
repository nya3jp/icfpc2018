#ifndef SOLVER_SUPPORT_ACTIONS_NOP_H
#define SOLVER_SUPPORT_ACTIONS_NOP_H

#include "solver/support/actions/base.h"

class MockAction : public Action {
 public:
  MockAction(const BotSet& bots) : Action(bots) {}
  MockAction(const MockAction& other) = delete;

  bool Decide(Commander* commander) override {
    return true;
  }
};

#endif //SOLVER_SUPPORT_ACTIONS_NOP_H
