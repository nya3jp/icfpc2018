#include "solver/manuals/fa002.h"

#include "solver/manuals/debug.h"
#include "solver/tasks/manual_assembler.h"

TaskPtr MakeManualFA002Task() {
  return MakeSequenceTask(
      // FA002
      Fill(Region(Point(8, 0, 7), Point(10, 0, 7))),
      Fill(Region(Point(7, 0, 8), Point(11, 0, 11))),

      Fill(Region(Point(9, 1, 9), Point(9, 8, 10))),

      Fill(Region(Point(7, 9, 7), Point(11, 17, 11))),

      Void(Region(Point(10, 9, 11), Point(11, 17, 11))),
      Void(Region(Point(7, 9, 10), Point(7, 17, 11))),

      Void(Region(Point(10, 9, 7), Point(11, 17, 7))),
      Void(Region(Point(11, 9, 8), Point(11, 17, 8))),

      Void(Region(Point(7, 9, 7), Point(9, 9, 7))),
      Void(Region(Point(7, 10, 7), Point(7, 10, 7))),

      Void(Region(Point(7, 13, 7), Point(7, 13, 7))),
      Void(Region(Point(7, 15, 7), Point(7, 16, 7))),

      MakePrintTask());
}
