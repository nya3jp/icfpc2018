#include "solver/manuals/example.h"

#include "solver/tasks/manual_assembler.h"

TaskPtr MakeManualExampleTask() {
  return MakeSequenceTask(
      Fill(Region(Point(6, 0, 6), Point(6, 6, 6))),
      Fill(Region(Point(3, 7, 3), Point(9, 8, 9))));
}
