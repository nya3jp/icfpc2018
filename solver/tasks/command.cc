#include "solver/tasks/command.h"

#include "glog/logging.h"

TaskPtr MakeCommandTask(int bot_id, Command command) {
  return MakeTask([=](Task::Commander* cmd) -> bool {
    bool success = cmd->Set(bot_id, command);
    CHECK(success);
    return true;
  });
}
