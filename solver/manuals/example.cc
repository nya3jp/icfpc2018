#include "solver/manuals/example.h"

#include <iostream>

#include "solver/tasks/manual_assembler.h"

namespace {

TaskPtr MakePrintTask() {
  return MakeTask([](Task::Commander* cmd) -> bool {
    const Matrix& target = cmd->field()->target();
    int resolution = target.Resolution();
    for (int y = 0; y < resolution; ++y) {
      std::cerr << "y = " << y << std::endl;
      std::cerr << " ";
      for (int x = 0; x < resolution; ++x) {
        std::cerr << x % 10;
      }
      std::cerr << std::endl;
      for (int z = resolution - 1; z >= 0; --z) {
        std::cerr << z % 10;
        for (int x = 0; x < resolution; ++x) {
          std::cerr << (target.Get(x, y, z) ? '#' : '.');
        }
        std::cerr << std::endl;
      }
      std::cerr << std::endl;
    }
    return true;
  });
}

}  // namespace

TaskPtr MakeManualExampleTask() {
  return MakeSequenceTask(
      //MakePrintTask(),
      Fill(Region(Point(6, 0, 6), Point(6, 6, 6))),
      Fill(Region(Point(3, 7, 3), Point(9, 8, 9))));
}
