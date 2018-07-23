#include "solver/manuals/example.h"

#include "solver/manuals/debug.h"
#include "solver/tasks/manual_assembler.h"

namespace {

}  // namespace

TaskPtr MakeManualExampleTask() {
  return MakeSequenceTask(
      // FA012
      Fill(Region(Point(7, 0, 7), Point(16, 0, 11))),

      Fill(Region(Point(5, 1, 7), Point(17, 1, 11))),
      Void(Region(Point(7, 1, 8), Point(16, 1, 10))),

      Fill(Region(Point(4, 2, 7), Point(17, 2, 11))),
      Void(Region(Point(5, 2, 8), Point(16, 2, 10))),

      Fill(Region(Point(3, 3, 7), Point(17, 3, 11))),
      Void(Region(Point(4, 3, 8), Point(9, 3, 10))),
      Void(Region(Point(15, 3, 8), Point(16, 3, 10))),

      Fill(Region(Point(3, 4, 7), Point(9, 4, 11))),
      Void(Region(Point(4, 4, 8), Point(8, 4, 10))),
      Fill(Region(Point(15, 4, 7), Point(17, 4, 11))),

      Fill(Region(Point(2, 5, 7), Point(8, 5, 11))),
      Void(Region(Point(3, 5, 8), Point(7, 5, 10))),
      Fill(Region(Point(17, 5, 7), Point(17, 5, 11))),

      Fill(Region(Point(2, 6, 7), Point(7, 6, 11))),
      Void(Region(Point(3, 6, 8), Point(6, 6, 10))),
      Fill(Region(Point(2, 7, 7), Point(7, 7, 11))),
      Void(Region(Point(3, 7, 8), Point(6, 7, 10))),
      Fill(Region(Point(2, 8, 7), Point(7, 8, 11))),
      Void(Region(Point(3, 8, 8), Point(6, 8, 10))),
      Fill(Region(Point(2, 9, 7), Point(7, 9, 11))),
      Void(Region(Point(3, 9, 8), Point(6, 9, 10))),
      Fill(Region(Point(2, 10, 7), Point(7, 10, 11))),
      Void(Region(Point(3, 10, 8), Point(6, 10, 10))),
      Fill(Region(Point(2, 11, 7), Point(7, 11, 11))),
      Void(Region(Point(3, 11, 8), Point(6, 11, 10))),
      Fill(Region(Point(2, 12, 7), Point(7, 12, 11))),
      Void(Region(Point(3, 12, 8), Point(6, 12, 10))),

      Fill(Region(Point(2, 13, 7), Point(8, 13, 11))),
      Void(Region(Point(3, 13, 8), Point(7, 13, 10))),

      Fill(Region(Point(3, 14, 7), Point(9, 14, 11))),
      Void(Region(Point(4, 14, 8), Point(8, 14, 10))),

      Fill(Region(Point(3, 15, 7), Point(17, 15, 11))),
      Void(Region(Point(4, 15, 8), Point(9, 15, 10))),
      Void(Region(Point(15, 15, 8), Point(16, 15, 10))),

      Fill(Region(Point(4, 16, 7), Point(17, 16, 11))),
      Void(Region(Point(5, 16, 8), Point(16, 16, 10))),

      Fill(Region(Point(5, 17, 7), Point(17, 17, 11))),
      Void(Region(Point(7, 17, 8), Point(16, 17, 10))),

      Fill(Region(Point(7, 18, 7), Point(17, 18, 11))),

      Fill(Region(Point(15, 14, 7), Point(17, 14, 11))),

      Fill(Region(Point(17, 13, 7), Point(17, 13, 11))),

      MakePrintTask());
}
