#ifndef SOLVER_SUPPORT_TASKS_NOP_H
#define SOLVER_SUPPORT_TASKS_NOP_H

#include "solver/support/tasks/base.h"

class MockTask : public Task {
 public:
  explicit MockTask(const BotSet& bots) : Task(bots) {}
  MockTask(const MockTask& other) = delete;

  bool Decide(Commander* commander) override {
    return true;
  }
};

#endif //SOLVER_SUPPORT_TASKS_NOP_H
