#ifndef SOLVER_SUPPORT_TASK_H
#define SOLVER_SUPPORT_TASK_H

#include <functional>
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
  MockTask() = default;
  MockTask(const MockTask& other) = delete;

  bool Decide(Commander* commander) override {
    return true;
  }
};

class FunctionTask : public Task {
 public:
  using FuncType = std::function<bool(TickExecutor::Commander*)>;

  explicit FunctionTask(FuncType func) : func_(std::move(func)) {}
  FunctionTask(const FunctionTask& other) = delete;

  bool Decide(Commander* commander) override {
    return func_(commander);
  }

 private:
  FuncType func_;
};

TaskPtr MakeTask(TaskPtr task);
TaskPtr MakeTask(Task* task);
TaskPtr MakeTask(std::function<bool(TickExecutor::Commander*)> func);

template<class T, class... Args>
void AppendTaskList(std::vector<TaskPtr>& tasks, T task_like, Args&&... args) {
  tasks.emplace_back(MakeTask(std::move(task_like)));
  AppendTaskList(tasks, std::forward<Args>(args)...);
};

template<class None = void>
void AppendTaskList(std::vector<TaskPtr>& tasks) {
}

template<class... Args>
std::vector<TaskPtr> MakeTaskList(Args&&... args) {
  std::vector<TaskPtr> tasks;
  AppendTaskList(tasks, std::forward<Args>(args)...);
  return tasks;
}

template<class... Args>
TaskPtr MakeSequenceTask(Args&&... args) {
  return MakeTask(new SequenceTask(MakeTaskList(std::forward<Args>(args)...)));
}

template<class... Args>
TaskPtr MakeBarrierTask(Args&&... args) {
  return MakeTask(new BarrierTask(MakeTaskList(std::forward<Args>(args)...)));
}

#endif //SOLVER_SUPPORT_TASK_H
