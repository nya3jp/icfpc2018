#include "solver/impls/naive.h"

#include <algorithm>
#include <utility>

#include "solver/data/geometry.h"

void NaiveSolver::Solve() {
  int n = model_->Resolution();

  // Move up to y=1.
  controller_.MoveDelta(Delta{0, 1, 0});

  // Iterate y=1, 2, 3, ...
  for (;;) {
    // Compute the bounding box in the current x-z plane.
    int min_x = n, min_z = n, max_x = -1, max_z = -1;
    for (int x = 0; x < n; ++x) {
      for (int z = 0; z < n; ++z) {
        if (model_->Get(x, Current().y - 1, z)) {
          min_x = std::min(min_x, x);
          min_z = std::min(min_z, z);
          max_x = std::max(max_x, x);
          max_z = std::max(max_z, z);
        }
      }
    }
    // If no filled voxel in the current x-z plane, we're done.
    if (max_x == -1) {
      break;
    }

    // Find the closest corner of the bounding box from the current position.
    int init_x = std::abs(min_x - Current().x) < std::abs(max_x - Current().x) ? min_x : max_x;
    int init_z = std::abs(min_z - Current().z) < std::abs(max_z - Current().z) ? min_z : max_z;

    // Determine x-z sweep parameters.
    int move_x = init_x == min_x ? 1 : -1;
    int move_z = init_z == min_z ? 1 : -1;
    int from_z = init_z;
    int to_z = init_z == min_z ? max_z : min_z;

    // Sweep the x-z plane.
    for (int x = init_x; min_x <= x && x <= max_x; x += move_x) {
      for (int z = from_z; min_z <= z && z <= max_z; z += move_z) {
        if (model_->Get(x, Current().y - 1, z)) {
          controller_.MoveTo(Point{x, Current().y, z});
          controller_.FillBelow();
        }
      }
      // Alternate Z iteration order.
      std::swap(from_z, to_z);
      move_z *= -1;
    }

    // Are we already at the top? Then it's done.
    if (Current().y == n - 1) {
      break;
    }

    // Go upward.
    controller_.MoveDelta(Delta{0, 1, 0});

    // We can always assume the output is grounded while y <= 1.
    if (Current().y == 2) {
      controller_.Flip();
    }
  }

  if (Current().y >= 2) {
    controller_.Flip();
  }

  // Move back to the origin.
  controller_.MoveTo(Point{0, Current().y, 0});
  controller_.MoveTo(Point{0, 0, 0});

  // Done!
  controller_.Halt();
}

const Point& NaiveSolver::Current() const {
  return controller_.current();
}
