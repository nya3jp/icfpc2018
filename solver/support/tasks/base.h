#ifndef SOLVER_SUPPORT_TASKS_BASE_H
#define SOLVER_SUPPORT_TASKS_BASE_H

#include <memory>
#include <vector>

#include "solver/support/tick_executor.h"

// Represents a task of one or more bots.
class Task {
 public:
  using Commander = TickExecutor::Commander;

  virtual ~Task() = default;

  // Decides the commands to perform for this tick.
  // This function should return true if the task is finished by this call of
  // Decide() or it has already finished.
  virtual bool Decide(Commander* commander) = 0;
};

using TaskPtr = std::unique_ptr<Task>;

class SequenceTask : public Task {
 public:
  explicit SequenceTask(std::vector<TaskPtr> tasks): tasks_(std::move(tasks)) {}
  SequenceTask(const SequenceTask& other) = delete;

  bool Decide(Commander* commander) override;

 private:
  std::vector<TaskPtr> tasks_;
  size_t index_ = 0;
};

class BarrierTask : public Task {
 public:
  explicit BarrierTask(std::vector<TaskPtr> tasks) : tasks_(std::move(tasks)) {}
  BarrierTask(const BarrierTask& other) = delete;

  bool Decide(Commander* commander) override;

 private:
  std::vector<TaskPtr> tasks_;
};

class MockTask : public Task {
 public:
  MockTask() {}
  MockTask(const MockTask& other) = delete;

  bool Decide(Commander* commander) override {
    return true;
  }
};

#endif //SOLVER_SUPPORT_TASKS_BASE_H
