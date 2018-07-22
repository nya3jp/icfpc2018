#include "solver/support/tasks/base.h"

#include "glog/logging.h"

bool SequenceTask::Decide(Commander* commander) {
  if (index_ >= tasks_.size()) {
    LOG(ERROR) << "Task exhausted";
  } else {
    bool done = tasks_[index_]->Decide(commander);
    if (done) {
      ++index_;
    }
  }
  return index_ >= tasks_.size();
}

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
