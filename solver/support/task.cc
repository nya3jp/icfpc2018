#include "solver/support/task.h"

#include <algorithm>

#include "glog/logging.h"

namespace {

class SequenceTask : public Task {
 public:
  explicit SequenceTask(std::vector<TaskPtr> tasks): tasks_(std::move(tasks)) {}
  SequenceTask(const SequenceTask& other) = delete;

  bool Decide(Commander* commander) override;

 private:
  std::vector<TaskPtr> tasks_;
  size_t index_ = 0;
};

bool SequenceTask::Decide(Commander* commander) {
  if (index_ < tasks_.size()) {
    bool done = tasks_[index_]->Decide(commander);
    if (done) {
      ++index_;
    }
  }
  return index_ >= tasks_.size();
}

class BarrierTask : public Task {
 public:
  explicit BarrierTask(std::vector<TaskPtr> tasks) : tasks_(std::move(tasks)) {}
  BarrierTask(const BarrierTask& other) = delete;

  bool Decide(Commander* commander) override;

 private:
  std::vector<TaskPtr> tasks_;
};

bool BarrierTask::Decide(Commander* commander) {
  bool done = true;
  for (auto& task : tasks_) {
    if (task) {
      if (task->Decide(commander)) {
        task.reset();
      } else {
        done = false;
      }
    }
  }
  return done;
}

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

class LazyFunctionTask : public Task {
 public:
  using FuncType = std::function<TaskPtr(TickExecutor::Commander*)>;

  explicit LazyFunctionTask(FuncType func) : func_(std::move(func)) {}
  LazyFunctionTask(const LazyFunctionTask& other) = delete;

  bool Decide(Commander* commander) override {
    if (!task_) {
      task_ = func_(commander);
    }
    return task_->Decide(commander);
  }

 private:
  FuncType func_;
  TaskPtr task_;
};

std::vector<TaskPtr> FilterNullTasks(std::vector<TaskPtr> tasks) {
  auto iter = std::remove_if(
      tasks.begin(), tasks.end(), [](const TaskPtr& task) -> bool { return !task; });
  tasks.erase(iter, tasks.end());
  return tasks;
}

}  // namespace

TaskPtr MakeTask(TaskPtr task) {
  return task;
}

TaskPtr MakeTask(Task* task) {
  return TaskPtr(task);
}

TaskPtr MakeTask(std::function<bool(TickExecutor::Commander*)> func) {
  return TaskPtr(new FunctionTask(std::move(func)));
}

TaskPtr MakeTask(std::function<TaskPtr(TickExecutor::Commander*)> func) {
  return TaskPtr(new LazyFunctionTask(std::move(func)));
}

TaskPtr MakeSequenceTask(std::vector<TaskPtr> subtasks) {
  return TaskPtr(new SequenceTask(FilterNullTasks(std::move(subtasks))));
}

TaskPtr MakeBarrierTask(std::vector<TaskPtr> subtasks) {
  return TaskPtr(new BarrierTask(FilterNullTasks(std::move(subtasks))));
}
