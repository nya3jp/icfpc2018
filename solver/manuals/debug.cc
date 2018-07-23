#include "solver/manuals/debug.h"

#include <iostream>

TaskPtr MakePrintTask() {

  // Uncomment this to debug
  return nullptr;

  return MakeTask([](Task::Commander* cmd) -> bool {
    const Matrix& matrix = cmd->field()->matrix();
    const Matrix& target = cmd->field()->target();
    int resolution = target.Resolution();
    for (int y = 0; y < resolution; ++y) {
      std::cerr << "y = " << y << std::endl;
      std::cerr << " ";
      for (int x = 0; x < resolution; ++x) {
        std::cerr << " " << x % 10;
      }
      std::cerr << "     ";
      for (int x = 0; x < resolution; ++x) {
        std::cerr << " " << x % 10;
      }
      std::cerr << std::endl;
      for (int z = resolution - 1; z >= 0; --z) {
        std::cerr << z % 10;
        for (int x = 0; x < resolution; ++x) {
          std::cerr << " " << (matrix.Get(x, y, z) ? '#' : '.');
        }
        std::cerr << "    " << z % 10;
        for (int x = 0; x < resolution; ++x) {
          std::cerr << " " << (target.Get(x, y, z) ? '#' : '.');
        }
        std::cerr << std::endl;
      }
      std::cerr << std::endl;
    }
    return true;
  });
}
