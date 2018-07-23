#ifndef SOLVER_TASKS_COMMAND_H
#define SOLVER_TASKS_COMMAND_H

#include "solver/data/command.h"
#include "solver/support/task.h"

TaskPtr MakeCommandTask(int bot_id, Command command);

#endif //SOLVER_TASKS_COMMAND_H
