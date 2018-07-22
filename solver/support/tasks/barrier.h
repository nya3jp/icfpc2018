#ifndef SOLVER_SUPPORT_TASKS_BARRIER_H
#define SOLVER_SUPPORT_TASKS_BARRIER_H

#include <memory>
#include <utility>
#include <vector>

#include "solver/support/tasks/base.h"

class BarrierTask : public Task {
 public:
  BarrierTask(const BotSet& bots, std::vector<std::unique_ptr<Task>> tasks)
      : Task(bots), tasks_(std::move(tasks)) {}
  BarrierTask(const BarrierTask& other) = delete;

  bool Decide(Commander* commander) override;

 private:
  std::vector<std::unique_ptr<Task>> tasks_;
};

#endif //SOLVER_SUPPORT_TASKS_BARRIER_H
