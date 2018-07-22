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

TaskPtr MakeTask(TaskPtr task);
TaskPtr MakeTask(Task* task);
TaskPtr MakeTask(std::function<bool(TickExecutor::Commander*)> func);
TaskPtr MakeTask(std::function<TaskPtr(TickExecutor::Commander*)> func);

template<class T, class... Args>
void AppendTaskList(std::vector<TaskPtr> &tasks, T task_like, Args &&... args) {
  tasks.emplace_back(MakeTask(std::move(task_like)));
  AppendTaskList(tasks, std::forward<Args>(args)...);
};

template<class None = void>
void AppendTaskList(std::vector<TaskPtr> &tasks) {
}

template<class... Args>
std::vector<TaskPtr> MakeTaskList(Args &&... args) {
  std::vector<TaskPtr> tasks;
  AppendTaskList(tasks, std::forward<Args>(args)...);
  return tasks;
}

TaskPtr MakeSequenceTask(std::vector<TaskPtr> subtasks);

template<class... Args>
TaskPtr MakeSequenceTask(Args&&... args) {
  return MakeSequenceTask(MakeTaskList(std::forward<Args>(args)...));
}

TaskPtr MakeBarrierTask(std::vector<TaskPtr> subtasks);

template<class... Args>
TaskPtr MakeBarrierTask(Args&&... args) {
  return MakeBarrierTask(MakeTaskList(std::forward<Args>(args)...));
}

#endif //SOLVER_SUPPORT_TASK_H
