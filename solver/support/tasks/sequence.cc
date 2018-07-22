#include "solver/support/tasks/sequence.h"

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
