#ifndef SOLVER_TASKS_MANUAL_ASSEMBLER_H
#define SOLVER_TASKS_MANUAL_ASSEMBLER_H

#include "solver/support/task.h"

TaskPtr MakeManualAssemblerTask(TaskPtr main_task);

// Manual functions.
TaskPtr Fill(Region region);

#endif //SOLVER_TASKS_MANUAL_ASSEMBLER_H
