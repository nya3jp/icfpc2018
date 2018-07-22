#include "solver/support/tasks/barrier.h"

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
