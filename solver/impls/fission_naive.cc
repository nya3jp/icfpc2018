#include "solver/impls/fission_naive.h"

#include <utility>

#include "glog/logging.h"

#include "solver/data/state.h"
#include "solver/support/task_executor.h"
#include "solver/support/task.h"

namespace {

class InitFissionTask : public Task {
 public:
  InitFissionTask() = default;
  InitFissionTask(const InitFissionTask& other) = delete;

  bool Decide(Commander* commander) override {
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
  }
};

class FinalFusionTask : public Task {
 public:
  FinalFusionTask() = default;
  FinalFusionTask(const FinalFusionTask& other) = delete;

  bool Decide(Commander* commander) override {
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
  }
};

class FlipTask : public Task {
 public:
  FlipTask() = default;
  FlipTask(const FlipTask &other) = delete;

  bool Decide(Commander *commander) override {
    bool success = commander->Set(commander->field()->bots().begin()->second.id(), Command::Flip());
    CHECK(success);
    return true;
  }
};

class NaiveMoveTask : public Task {
 public:
  NaiveMoveTask(int bot_id, const Point& destination)
      : bot_id_(bot_id), destination_(destination) {}
  NaiveMoveTask(const NaiveMoveTask& other) = delete;

  bool Decide(Commander* commander) override {
    const Point position =
        commander->field()->bots().find(bot_id_)->second.position();
    const Delta delta = destination_ - position;

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
      bool success = commander->Set(bot_id_, Command::LMove(linears[0], linears[1]));
      CHECK(success);
      return true;
    }

    if (linears.empty()) {
      return true;
    }

    LinearDelta move{
        linears[0].axis,
        std::max(std::min(linears[0].delta, LONG_LEN), -LONG_LEN)};
    bool success = commander->Set(bot_id_, Command::SMove(move));
    CHECK(success);
    return move == linears[0];
  }

 private:
  const int bot_id_;
  const Point destination_;
};

class RegionAssignTask : public Task {
 public:
  explicit RegionAssignTask(std::vector<Region> regions) : regions_(std::move(regions)) {}
  RegionAssignTask(const RegionAssignTask& other) = delete;

  bool Decide(Commander* commander) override {
    if (!task_) {
      const auto& bots = commander->field()->bots();
      CHECK_EQ(bots.size(), regions_.size());
      std::vector<TaskPtr> subtasks;
      auto iter = regions_.cbegin();
      for (const auto& pair : bots) {
        subtasks.emplace_back(new NaiveMoveTask(pair.first, iter->mini));
        ++iter;
      }
      // Reverse the order to avoid conflicts.
      std::reverse(subtasks.begin(), subtasks.end());
      task_ = MakeTask(new SequenceTask(std::move(subtasks)));
    }
    return task_->Decide(commander);
  }

 private:
  std::vector<Region> regions_;
  TaskPtr task_;
};

class RegionUnassignTask : public Task {
 public:
  RegionUnassignTask() = default;
  RegionUnassignTask(const RegionUnassignTask& other) = delete;

  bool Decide(Commander* commander) override {
    if (!task_) {
      const auto& bots = commander->field()->bots();
      std::vector<TaskPtr> subtasks;
      for (const auto &pair : bots) {
        subtasks.emplace_back(
            new NaiveMoveTask(pair.first, Point(pair.first, 0, 0)));
      }
      task_ = MakeTask(new SequenceTask(std::move(subtasks)));
    }
    return task_->Decide(commander);
  }

 private:
  TaskPtr task_;
};

class FillTask : public Task {
 public:
  FillTask(int bot_id, const Delta &nd) : bot_id_(bot_id), nd_(nd) {}
  FillTask(const FillTask &other) = delete;

  bool Decide(Commander* commander) override {
    commander->Set(bot_id_, Command::Fill(nd_));
    return true;
  }

 private:
  const int bot_id_;
  const Delta nd_;
};

class HaltTask : public Task {
 public:
  HaltTask() = default;
  HaltTask(const HaltTask &other) = delete;

  bool Decide(Commander* commander) override {
    commander->Set(0, Command::Halt());
    return true;
  }
};

class PrintTask : public Task {
 public:
  PrintTask(int bot_id, const Region& region) : bot_id_(bot_id), region_(region) {}
  PrintTask(const PrintTask& other) = delete;

  bool Decide(Commander* commander) override {
    if (!task_) {
      const Matrix& target = commander->field()->target();
      std::vector<TaskPtr> plan;

      Point pos = commander->field()->bots().find(bot_id_)->second.position();
      CHECK_EQ(pos, region_.mini);

      // Iterate y=1, 2, 3, ...
      for (int scan_y = 1; scan_y < region_.maxi.y; ++scan_y) {
        // Compute the bounding box in the current x-z plane.
        int min_x = region_.maxi.x + 1;
        int min_z = region_.maxi.z + 1;
        int max_x = region_.mini.x - 1;
        int max_z = region_.mini.z - 1;
        for (int x = region_.mini.x; x <= region_.maxi.x; ++x) {
          for (int z = region_.mini.z; z <= region_.maxi.z; ++z) {
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
        plan.emplace_back(new NaiveMoveTask(bot_id_, pos));

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
              plan.emplace_back(new NaiveMoveTask(bot_id_, pos));
              plan.emplace_back(new FillTask(bot_id_, Delta(0, -1, 0)));
            }
          }
          // Alternate Z iteration order.
          std::swap(from_z, to_z);
          move_z *= -1;
        }
      }

      // Go back to the home position.
      pos.y += 1;
      plan.emplace_back(new NaiveMoveTask(bot_id_, pos));
      pos.z = 0;
      plan.emplace_back(new NaiveMoveTask(bot_id_, pos));
      pos = region_.mini;
      plan.emplace_back(new NaiveMoveTask(bot_id_, pos));

      task_ = MakeTask(new SequenceTask(std::move(plan)));
    }
    return task_->Decide(commander);
  }

 private:
  const int bot_id_;
  const Region region_;
  TaskPtr task_;
};

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

class MainTask : public Task {
 public:
  MainTask() = default;
  MainTask(const MainTask& other) = delete;

  bool Decide(Commander* commander) override {
    if (!task_) {
      auto regions = ComputeAssignedRegions(*commander->field());
      std::vector<TaskPtr> print_tasks;
      {
        auto iter = regions.cbegin();
        for (const auto& pair : commander->field()->bots()) {
          print_tasks.emplace_back(new PrintTask(pair.first, *iter));
          ++iter;
        }
      }
      task_ = MakeSequenceTask(
          new RegionAssignTask(regions),
          new FlipTask(),
          new BarrierTask(std::move(print_tasks)),
          new FlipTask(),
          new RegionUnassignTask());
    }
    return task_->Decide(commander);
  }

 private:
  TaskPtr task_;
};

TaskPtr CreateMasterTask() {
  return MakeSequenceTask(
      new InitFissionTask(),
      new MainTask(),
      new FinalFusionTask(),
      new HaltTask());
}

}  // namespace

FissionNaiveSolver::FissionNaiveSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
}

void FissionNaiveSolver::Solve() {
  TaskExecutor executor(CreateMasterTask());
  executor.Run(&field_, writer_);
  CHECK(field_.IsHalted()) << "Not halted, but expected";
}
