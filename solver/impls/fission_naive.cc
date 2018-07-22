#include "solver/impls/fission_naive.h"

#include <utility>

#include "glog/logging.h"

#include "solver/data/state.h"
#include "solver/support/task_executor.h"
#include "solver/support/task.h"

namespace {

TaskPtr MakeInitFissionTask() {
  return MakeTask([](Task::Commander *commander) -> bool {
    int resolution = commander->field()->matrix().Resolution();
    const auto& bots = commander->field()->bots();
    int num_bots = static_cast<int>(bots.size());

    for (int bot_id = 0; bot_id < num_bots; ++bot_id) {
      const auto& bot = bots.find(bot_id)->second;
      CHECK_EQ(bot.position(), Point(bot_id, 0, 0));
      if (bot_id < num_bots - 1) {
        CHECK_EQ(bot.seeds(), 0);
      }
    }

    const auto& last_bot = bots.find(num_bots - 1)->second;
    int seed_count = __builtin_popcountl(last_bot.seeds());
    CHECK_GE(seed_count, 1) << last_bot.seeds();

    bool success = commander->Set(
        num_bots - 1, Command::Fission(Delta(1, 0, 0), seed_count - 1));
    CHECK(success);

    return seed_count == 1 || num_bots == resolution - 1;
  });
}

TaskPtr MakeFinalFusionTask() {
  return MakeTask([](Task::Commander* commander) -> bool {
    const auto& bots = commander->field()->bots();
    int num_bots = static_cast<int>(bots.size());
    CHECK(num_bots >= 2);

    for (int bot_id = 0; bot_id < num_bots; ++bot_id) {
      const auto& bot = bots.find(bot_id)->second;
      CHECK_EQ(bot.position(), Point(bot_id, 0, 0));
    }

    const auto& primary_bot = bots.find(num_bots - 2)->second;
    const auto& secondary = bots.find(num_bots - 1)->second;

    bool primary_success = commander->Set(
        num_bots - 2, Command::FusionP(Delta(1, 0, 0)));
    bool secondary_success = commander->Set(
        num_bots - 1, Command::FusionS(Delta(-1, 0, 0)));
    CHECK(primary_success);
    CHECK(secondary_success);

    return num_bots == 2;
  });
}

TaskPtr MakeFlipTask() {
  return MakeTask([](Task::Commander* commander) -> bool {
    commander->Set(0, Command::Flip());
    return true;
  });
}

TaskPtr MakeNaiveMoveTask(int bot_id, Point destination) {
  return MakeTask([=](Task::Commander* commander) -> bool {
    const Point position =
        commander->field()->bots().find(bot_id)->second.position();
    const Delta delta = destination - position;

    std::vector<LinearDelta> linears;
    if (delta.dx != 0) {
      linears.emplace_back(Axis::X, delta.dx);
    }
    if (delta.dy != 0) {
      linears.emplace_back(Axis::Y, delta.dy);
    }
    if (delta.dz != 0) {
      linears.emplace_back(Axis::Z, delta.dz);
    }

    if (linears.size() == 2 &&
        std::max(std::abs(linears[0].delta), std::abs(linears[1].delta)) <= SHORT_LEN) {
      bool success = commander->Set(bot_id, Command::LMove(linears[0], linears[1]));
      CHECK(success);
      return true;
    }

    if (linears.empty()) {
      return true;
    }

    LinearDelta move{
        linears[0].axis,
        std::max(std::min(linears[0].delta, LONG_LEN), -LONG_LEN)};
    bool success = commander->Set(bot_id, Command::SMove(move));
    CHECK(success);
    return linears.size() == 1 && move == linears[0];
  });
}

TaskPtr MakeRegionAssignTask(std::vector<Region> regions) {
  return MakeTask([=](Task::Commander* commander) -> TaskPtr {
    const auto& bots = commander->field()->bots();
    CHECK_EQ(bots.size(), regions.size());
    std::vector<TaskPtr> subtasks;
    auto iter = regions.cbegin();
    for (const auto& pair : bots) {
      subtasks.emplace_back(MakeNaiveMoveTask(pair.first, iter->mini));
      ++iter;
    }
    // Reverse the order to avoid conflicts.
    std::reverse(subtasks.begin(), subtasks.end());
    return MakeSequenceTask(std::move(subtasks));
  });
}

TaskPtr MakeRegionUnassignTask() {
  return MakeTask([](Task::Commander* commander) -> TaskPtr {
    const auto& bots = commander->field()->bots();
    std::vector<TaskPtr> subtasks;
    for (const auto &pair : bots) {
      subtasks.emplace_back(
          MakeNaiveMoveTask(pair.first, Point(pair.first, 0, 0)));
    }
    return MakeSequenceTask(std::move(subtasks));
  });
}

TaskPtr MakeFillTask(int bot_id, Delta nd) {
  return MakeTask([=](Task::Commander* commander) -> bool {
    commander->Set(bot_id, Command::Fill(nd));
    return true;
  });
}

TaskPtr MakeHaltTask() {
  return MakeTask([](Task::Commander* commander) -> bool {
    commander->Set(0, Command::Halt());
    return true;
  });
}

TaskPtr MakePrintTask(int bot_id, Region region) {
  return MakeTask([=](Task::Commander* commander) -> TaskPtr {
    const Matrix& target = commander->field()->target();
    std::vector<TaskPtr> plan;

    Point pos = commander->field()->bots().find(bot_id)->second.position();
    CHECK_EQ(pos, region.mini);

    // Iterate y=1, 2, 3, ...
    for (int scan_y = 1; scan_y <= region.maxi.y; ++scan_y) {
      // Compute the bounding box in the current x-z plane.
      int min_x = region.maxi.x + 1;
      int min_z = region.maxi.z + 1;
      int max_x = region.mini.x - 1;
      int max_z = region.mini.z - 1;
      for (int x = region.mini.x; x <= region.maxi.x; ++x) {
        for (int z = region.mini.z; z <= region.maxi.z; ++z) {
          if (target.Get(x, scan_y - 1, z)) {
            min_x = std::min(min_x, x);
            min_z = std::min(min_z, z);
            max_x = std::max(max_x, x);
            max_z = std::max(max_z, z);
          }
        }
      }
      // If no filled voxel in the current x-z plane, skip to the next y.
      if (max_x < min_x) {
        continue;
      }

      // We have something to fill in this level, go up.
      pos.y = scan_y;
      plan.emplace_back(MakeNaiveMoveTask(bot_id, pos));

      // Find the closest corner of the bounding box from the current position.
      int init_x =
          std::abs(min_x - pos.x) < std::abs(max_x - pos.x) ? min_x : max_x;
      int init_z =
          std::abs(min_z - pos.z) < std::abs(max_z - pos.z) ? min_z : max_z;

      // Determine x-z sweep parameters.
      int move_x = init_x == min_x ? 1 : -1;
      int move_z = init_z == min_z ? 1 : -1;
      int from_z = init_z;
      int to_z = init_z == min_z ? max_z : min_z;

      // Sweep the x-z plane.
      for (int x = init_x; min_x <= x && x <= max_x; x += move_x) {
        for (int z = from_z; min_z <= z && z <= max_z; z += move_z) {
          if (target.Get(x, scan_y - 1, z)) {
            pos = Point(x, scan_y, z);
            plan.emplace_back(MakeNaiveMoveTask(bot_id, pos));
            plan.emplace_back(MakeFillTask(bot_id, Delta(0, -1, 0)));
          }
        }
        // Alternate Z iteration order.
        std::swap(from_z, to_z);
        move_z *= -1;
      }
    }

    // Go back to the home position.
    if (pos.y < region.maxi.y - 1) {
      pos.y += 1;
      plan.emplace_back(MakeNaiveMoveTask(bot_id, pos));
    }
    pos.z = 0;
    plan.emplace_back(MakeNaiveMoveTask(bot_id, pos));
    pos = region.mini;
    plan.emplace_back(MakeNaiveMoveTask(bot_id, pos));

    return MakeSequenceTask(std::move(plan));
  });
}

std::vector<Region> ComputeAssignedRegions(const FieldState& field) {
  std::vector<Region> regions;
  int num_bots = static_cast<int>(field.bots().size());
  int resolution = field.matrix().Resolution();
  for (int i = 0; i < num_bots; ++i) {
    int min_x = resolution * i / num_bots;
    int max_x = resolution * (i + 1) / num_bots - 1;
    regions.emplace_back(Point(min_x, 0, 0), Point(max_x, resolution - 1, resolution - 1));
  }
  return regions;
}

TaskPtr MakeMainTask() {
  return MakeTask([](Task::Commander *commander) -> TaskPtr {
    auto regions = ComputeAssignedRegions(*commander->field());
    std::vector<TaskPtr> print_tasks;
    {
      auto iter = regions.cbegin();
      for (const auto &pair : commander->field()->bots()) {
        print_tasks.emplace_back(MakePrintTask(pair.first, *iter));
        ++iter;
      }
    }
    return MakeSequenceTask(
        MakeRegionAssignTask(regions),
        MakeFlipTask(),
        MakeBarrierTask(std::move(print_tasks)),
        MakeFlipTask(),
        MakeRegionUnassignTask());
  });
}

TaskPtr MakeMasterTask() {
  return MakeSequenceTask(
      MakeInitFissionTask(),
      MakeMainTask(),
      MakeFinalFusionTask(),
      MakeHaltTask());
}

}  // namespace

FissionNaiveSolver::FissionNaiveSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
}

void FissionNaiveSolver::Solve() {
  TaskExecutor executor(MakeMasterTask());
  executor.Run(&field_, writer_);
  CHECK(field_.IsHalted()) << "Not halted, but expected";
}
