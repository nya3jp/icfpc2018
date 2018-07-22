#ifndef SOLVER_SUPPORT_TASKS_SEQUENCE_H
#define SOLVER_SUPPORT_TASKS_SEQUENCE_H

#include <memory>
#include <utility>
#include <vector>

#include "solver/support/tasks/base.h"

class SequenceTask : public Task {
 public:
  SequenceTask(const BotSet& bots, std::vector<std::unique_ptr<Task>> tasks)
      : Task(bots), tasks_(std::move(tasks)) {}
  SequenceTask(const SequenceTask& other) = delete;

  bool Decide(Commander* commander) override;

 private:
  std::vector<std::unique_ptr<Task>> tasks_;
  size_t index_ = 0;
};

#endif //SOLVER_SUPPORT_TASKS_SEQUENCE_H
