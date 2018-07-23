#include "solver/tasks/line_assembler.h"

#include <algorithm>
#include <functional>
#include <set>

#include "glog/logging.h"
#include "gflags/gflags.h"

#include "solver/support/union_find.h"
#include "solver/tasks/command.h"

#define FIELD (*cmd->field())
#define MATRIX (FIELD.matrix())
#define TARGET (FIELD.target())
#define BOTS (FIELD.bots())
#define BOT(bot_id) (BOTS.find(bot_id)->second)

DEFINE_bool(line_assembler_flip_xz, false, "Scan by X axis");
DEFINE_int32(line_assembler_x_divs, 5, "X division");
DEFINE_int32(line_assembler_z_divs, 4, "Z division");

namespace {

inline int CapAbs(int delta, int cap) {
  return std::max(std::min(delta, cap), -cap);
}

inline int Sign(int delta) {
  return delta == 0 ? 0 : delta > 0 ? 1 : -1;
}

Region GetBoundingBox(const Matrix& matrix) {
  int resolution = matrix.Resolution();
  Point mini(resolution, resolution, resolution);
  Point maxi(-1, -1, -1);
  for (int x = 0; x < resolution; ++x) {
    for (int y = 0; y < resolution; ++y) {
      for (int z = 0; z < resolution; ++z) {
        if (matrix.Get(x, y, z)) {
          mini.x = std::min(mini.x, x);
          mini.y = std::min(mini.y, y);
          mini.z = std::min(mini.z, z);
          maxi.x = std::max(maxi.x, x);
          maxi.y = std::max(maxi.y, y);
          maxi.z = std::max(maxi.z, z);
        }
      }
    }
  }
  return Region(mini, maxi);
}

std::vector<Region> ComputeRegions(Task::Commander* cmd) {
  constexpr int MIN_WIDTH = 2;
  int x_divs = FLAGS_line_assembler_x_divs;
  int z_divs = FLAGS_line_assembler_z_divs;

  const int resolution = TARGET.Resolution();
  Region bound = GetBoundingBox(TARGET);

  // Extend bounding to afford regions.
  while (bound.maxi.x - bound.mini.x < MIN_WIDTH * x_divs) {
    if (bound.mini.x > 0) {
      --bound.mini.x;
    } else if (bound.maxi.x < resolution - 1) {
      ++bound.maxi.x;
    } else {
      LOG(FATAL) << "Resolution is too small";
    }
  }
  while (bound.maxi.z - bound.mini.z < MIN_WIDTH * z_divs) {
    if (bound.mini.z > 0) {
      --bound.mini.z;
    } else if (bound.maxi.z < resolution - 1) {
      ++bound.maxi.z;
    } else {
      LOG(FATAL) << "Resolution is too small";
    }
  }

  std::vector<Region> regions;
  for (int ix = 0; ix < x_divs; ++ix) {
    for (int iz = 0; iz < z_divs; ++iz) {
      regions.emplace_back(
          Point(bound.mini.x + (bound.maxi.x - bound.mini.x + 1) * ix / x_divs,
                0,
                bound.mini.z + (bound.maxi.z - bound.mini.z + 1) * iz / z_divs),
          Point(bound.mini.x + (bound.maxi.x - bound.mini.x + 1) * (ix + 1) / x_divs - 1,
                resolution - 1,
                bound.mini.z + (bound.maxi.z - bound.mini.z + 1) * (iz + 1) / z_divs - 1));
    }
  }

  return regions;
}

TaskPtr MakeGreedyMoveTask(int bot_id, Point destination) {
  int stuck = 0;
  bool detoured = false;
  return MakeTask([=](Task::Commander* cmd) mutable -> bool {
    Point position = BOT(bot_id).position();
    Delta delta = destination - position;
    if (delta.IsZero()) {
      return true;
    }

    bool can_detour = !detoured;
    detoured = false;

    //LOG(INFO) << "Bot " << bot_id << ": " << position << " -> " << destination;
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
    CHECK(!linears.empty());

    // Try LMove
    if (linears.size() == 2 &&
        std::max(std::abs(linears[0].delta), std::abs(linears[1].delta)) <= SHORT_LEN) {
      if (cmd->Set(bot_id, Command::LMove(linears[0], linears[1]))) {
        //LOG(INFO) << ">>> LMove " << linears[0] << " " << linears[1];
        return true;
      }
    }

    // Try SMove
    for (int cap = std::min(LONG_LEN, delta.Chessboard()); cap > 0; --cap) {
      // TODO(nya): Randomize order
      for (auto axis : {Axis::X, Axis::Y, Axis::Z}) {
        int component = delta.GetAxis(axis);
        if (std::abs(component) < cap) {
          continue;
        }
        LinearDelta move{axis, CapAbs(component, cap)};
        if (cmd->Set(bot_id, Command::SMove(move))) {
          //LOG(INFO) << ">>> SMove " << move;
          return linears.size() == 1 && std::abs(component) == cap;
        }
      }
    }

    if (!can_detour) {
      return false;
    }

    // Try detours
    for (auto detour_axis : {Axis::Y, Axis::Z, Axis::X}) {
      if (delta.GetAxis(detour_axis) != 0) {
        continue;
      }
      for (int detour_len = 1; detour_len <= SHORT_LEN; ++detour_len) {
        for (int detour_delta : {detour_len, -detour_len}) {
          LinearDelta detour{detour_axis, detour_delta};
          Point corner = position + detour.ToDelta();
          if (!TARGET.Contains(corner)) {
            continue;
          }
          for (Axis forward_axis : {Axis::X, Axis::Y, Axis::Z}) {
            if (delta.GetAxis(forward_axis) == 0) {
              continue;
            }
            for (int forward_delta = CapAbs(delta.GetAxis(forward_axis), SHORT_LEN);
                forward_delta != 0;
                forward_delta -= Sign(forward_delta)) {
              LinearDelta forward{forward_axis, forward_delta};
              if (cmd->Set(bot_id, Command::LMove(detour, forward))) {
                //LOG(INFO) << ">>> detour LMove " << detour << " " << forward;
                detoured = true;
                return false;
              }
            }
          }
        }
      }
    }

    CHECK(++stuck < 40) << "Bot " << bot_id << " stuck: " << position << " -> " << destination;
    return false;
  });
}

TaskPtr MakeLinearFissionTask(int bot_id, int repeat) {
  if (repeat == 0) {
    return nullptr;
  }
  return MakeTask([=](Task::Commander *cmd) -> TaskPtr {
    uint64_t seeds = BOT(bot_id).seeds();
    CHECK(seeds);
    int num_seeds = __builtin_popcountl(seeds);
    int next_bot_id = __builtin_ctzl(seeds);
    TaskPtr task =
        MakeCommandTask(bot_id, Command::Fission(Delta(1, 0, 0), num_seeds - 1));
    if (num_seeds == 1) {
      return task;
    }
    return MakeSequenceTask(std::move(task), MakeLinearFissionTask(next_bot_id, repeat - 1));
  });
}

TaskPtr MakeFissionTask() {
  int num_bots = FLAGS_line_assembler_x_divs * FLAGS_line_assembler_z_divs * 2;
  CHECK_LE(num_bots, 40);
  if (num_bots == 2) {
    return MakeCommandTask(0, Command::Fission(Delta(0, 0, 1), 0));
  }
  if (num_bots == 4) {
    return MakeSequenceTask(
      MakeCommandTask(0, Command::Fission(Delta(0, 0, 1), 1)),  // 0: [1, 40) -> 1
      MakeBarrierTask(
          MakeCommandTask(0, Command::Fission(Delta(0, 1, 0), 0)),   // 0: [3, 40) -> 21
          MakeCommandTask(1, Command::Fission(Delta(0, 1, 0), 0))));  // 1: [2, 3) -> 2
  }
  CHECK_EQ(num_bots % 2, 0);
  int m = num_bots / 4;
  int rest = (num_bots % 4) / 2;
  return MakeSequenceTask(
      MakeCommandTask(0, Command::Fission(Delta(0, 0, 1), 3 * m - 1 + rest)),  // 0: [1, 40) -> 1
      MakeCommandTask(1, Command::Fission(Delta(0, 1, 0), 2 * m - 1)),  // 1: [2, 3m+1+rest] -> 2
      MakeCommandTask(2, Command::Fission(Delta(0, 0, -1), m - 1)),  // 2: [3, 2m+1] -> 3
      MakeBarrierTask(
          MakeLinearFissionTask(0, m - 1 + rest),
          MakeLinearFissionTask(1, m - 1 + rest),
          MakeLinearFissionTask(2, m - 1),
          MakeLinearFissionTask(3, m - 1)));
}

TaskPtr MakeGoCeilingTask() {
  return MakeTask([=](Task::Commander *cmd) -> TaskPtr {
    // Assumes all bots have distinct x/z coordinates.
    const int resolution = TARGET.Resolution();
    std::vector<TaskPtr> subtasks;
    const auto& bots = BOTS;
    for (const auto& pair : bots) {
      const auto& bot = pair.second;
      if (bot.position().y != resolution - 1) {
        Point ceiling = bot.position();
        ceiling.y = resolution - 1;
        subtasks.emplace_back(MakeGreedyMoveTask(bot.id(), ceiling));
      }
    }
    return MakeBarrierTask(std::move(subtasks));
  });
}

TaskPtr MakeRoughFusionTask() {
  return MakeTask([](Task::Commander *cmd) -> bool {
    const auto& bots = BOTS;
    int num_remaining_bots = static_cast<int>(bots.size());

    std::set<int> fusion_ids;
    for (auto i = bots.begin(); i != bots.end(); ++i) {
      if (fusion_ids.count(i->first) > 0) {
        continue;
      }
      Point pi = i->second.position();
      auto j = i;
      for (++j; j != bots.end(); ++j) {
        if (fusion_ids.count(j->first) > 0) {
          continue;
        }
        Point pj = j->second.position();
        Delta d = pj - pi;
        if (d.IsNear()) {
          bool i_success = cmd->Set(i->first, Command::FusionP(d));
          bool j_success = cmd->Set(j->first, Command::FusionS(Delta() - d));
          CHECK(i_success);
          CHECK(j_success);
          fusion_ids.insert(i->first);
          fusion_ids.insert(j->first);
          --num_remaining_bots;
          break;
        }
      }
    }

    const Point meeting_point(0, TARGET.Resolution() - 1, 0);
    for (auto i = bots.begin(); i != bots.end(); ++i) {
      if (fusion_ids.count(i->first) > 0) {
        continue;
      }
      MakeGreedyMoveTask(i->first, meeting_point)->Decide(cmd);
    }

    return num_remaining_bots == 1;
  });
}

TaskPtr MakeGoOriginTask() {
  return MakeTask([=](Task::Commander *cmd) -> TaskPtr {
    CHECK_EQ(1, BOTS.size());
    const int bot_id = BOTS.begin()->first;
    return MakeGreedyMoveTask(bot_id, Point());
  });
}

TaskPtr MakeFinishTask() {
  return MakeTask([=](Task::Commander *cmd) -> TaskPtr {
    return MakeSequenceTask(
        MakeGoCeilingTask(),
        MakeRoughFusionTask(),
        MakeGoOriginTask());
  });
}

TaskPtr MakeScatterToRegionsTask(std::vector<Region> regions) {
  return MakeTask([=](Task::Commander* cmd) -> TaskPtr {
    std::vector<TaskPtr> subtasks;
    for (int index_region = 0; index_region < static_cast<int>(regions.size()); ++index_region) {
      const Region& region = regions[index_region];
      subtasks.emplace_back(MakeGreedyMoveTask(index_region * 2 + 0, region.mini));
      subtasks.emplace_back(MakeGreedyMoveTask(index_region * 2 + 1, region.mini + Delta(0, 0, 1)));
    }
    return MakeBarrierTask(std::move(subtasks));
  });
}

TaskPtr MakeRegionLineAssembleTask(int near_bot_id, int far_bot_id, Region region) {
  return MakeTask([=](Task::Commander* cmd) -> TaskPtr {
    const int resolution = TARGET.Resolution();
    std::vector<TaskPtr> plan;

    Point near_bot_pos = BOT(near_bot_id).position();
    Point far_bot_pos = BOT(far_bot_id).position();
    CHECK_EQ(near_bot_pos.y, far_bot_pos.y);

    auto MaybeGoUp = [&](int y) {
      if (near_bot_pos.y != y) {
        near_bot_pos.y = y;
        far_bot_pos.y = y;
        plan.emplace_back(MakeGreedyMoveTask(near_bot_id, near_bot_pos));
        plan.emplace_back(MakeGreedyMoveTask(far_bot_id, far_bot_pos));
      }
    };

    auto DoPointFill = [&](Point destination) {
      MaybeGoUp(destination.y);

      if (destination == far_bot_pos) {
        plan.emplace_back(MakeCommandTask(far_bot_id, Command::Fill(Delta(0, -1, 0))));
      } else {
        if (destination != near_bot_pos) {
          near_bot_pos = destination;
          plan.emplace_back(MakeGreedyMoveTask(near_bot_id, near_bot_pos));
        }
        CHECK_EQ(near_bot_pos, destination);
        plan.emplace_back(MakeCommandTask(near_bot_id, Command::Fill(Delta(0, -1, 0))));
      }
    };

    auto DoLineFill = [&](Point near_destination, Point far_destination) {
      MaybeGoUp(near_destination.y);

      std::vector<TaskPtr> subtasks;
      if (near_bot_pos != near_destination) {
        near_bot_pos = near_destination;
        subtasks.emplace_back(MakeGreedyMoveTask(near_bot_id, near_bot_pos));
      }
      if (far_bot_pos != far_destination) {
        far_bot_pos = far_destination;
        subtasks.emplace_back(MakeGreedyMoveTask(far_bot_id, far_bot_pos));
      }

      if (!subtasks.empty()) {
        plan.emplace_back(MakeBarrierTask(std::move(subtasks)));
      }
      plan.emplace_back(MakeBarrierTask(
          MakeCommandTask(
              near_bot_id,
              Command::GFill(Delta(0, -1, 0), far_destination - near_destination)),
          MakeCommandTask(
              far_bot_id,
              Command::GFill(Delta(0, -1, 0), near_destination - far_destination))));
    };

    for (int y = 1; y < resolution; ++y) {
      if (FLAGS_line_assembler_flip_xz) {
        for (int z = region.mini.z; z <= region.maxi.z; ++z) {
          int near_x = region.mini.x;
          while (near_x <= region.maxi.x) {
            while (near_x <= region.maxi.x && !TARGET.Get(near_x, y - 1, z)) {
              ++near_x;
            }
            if (near_x > region.maxi.x) {
              continue;
            }
            int far_x = near_x;
            while (far_x < region.maxi.x &&
                   far_x - near_x + 1 < FAR_LEN &&
                   TARGET.Get(far_x + 1, y - 1, z)) {
              ++far_x;
            }
            if (near_x == far_x) {
              DoPointFill(Point(near_x, y, z));
            } else {
              DoLineFill(Point(near_x, y, z), Point(far_x, y, z));
            }
            near_x = far_x + 1;
          }
        }
      } else {
        for (int x = region.mini.x; x <= region.maxi.x; ++x) {
          int near_z = region.mini.z;
          while (near_z <= region.maxi.z) {
            while (near_z <= region.maxi.z && !TARGET.Get(x, y - 1, near_z)) {
              ++near_z;
            }
            if (near_z > region.maxi.z) {
              continue;
            }
            int far_z = near_z;
            while (far_z < region.maxi.z &&
                   far_z - near_z + 1 < FAR_LEN &&
                   TARGET.Get(x, y - 1, far_z + 1)) {
              ++far_z;
            }
            if (near_z == far_z) {
              DoPointFill(Point(x, y, near_z));
            } else {
              DoLineFill(Point(x, y, near_z), Point(x, y, far_z));
            }
            near_z = far_z + 1;
          }
        }
      }
    }
    return MakeSequenceTask(std::move(plan));
  });
}

TaskPtr MakeParallelLineAssembleTask(std::vector<Region> regions) {
  return MakeTask([=](Task::Commander* cmd) -> TaskPtr {
    std::vector<TaskPtr> subtasks;
    for (int index_region = 0; index_region < static_cast<int>(regions.size()); ++index_region) {
      const Region& region = regions[index_region];
      subtasks.emplace_back(
          MakeRegionLineAssembleTask(index_region * 2 + 0,
                                     index_region * 2 + 1,
                                     region));
    }
    return MakeBarrierTask(std::move(subtasks));
  });
}

class LazyFlipMiddlewareTask : public Task {
 public:
  LazyFlipMiddlewareTask(TaskPtr task)
      : task_(std::move(task)), GROUND(-999, -999, -999) {
    union_find_.Add(GROUND);
  }
  LazyFlipMiddlewareTask(const LazyFlipMiddlewareTask& other) = delete;

  bool Decide(Commander* cmd) override {
    if (!holdback_commands_.empty()) {
      for (const auto& pair : holdback_commands_) {
        bool success = cmd->Set(pair.first, pair.second);
        CHECK(success);
      }
      holdback_commands_.clear();
      return holdback_done_;
    }

    const int any_bot_id = BOTS.begin()->first;

    if (done_deferred_) {
      CHECK(harmonic_);
      bool success = cmd->Set(any_bot_id, Command::Flip());
      CHECK(success);
      return true;
    }

    if (harmonic_ && IsGrounded()) {
      bool success = cmd->Set(any_bot_id, Command::Flip());
      CHECK(success);
      harmonic_ = false;
      return false;
    }

    Commander copy_cmd = cmd->Copy();
    bool done = task_->Decide(&copy_cmd);

    UpdateUnionFind(copy_cmd.GetFills());

    if (!harmonic_ && !IsGrounded()) {
      holdback_commands_ = copy_cmd.commands();
      holdback_done_ = done;
      bool success = cmd->Set(any_bot_id, Command::Flip());
      CHECK(success);
      harmonic_ = true;
      return false;
    }

    std::swap(*cmd, copy_cmd);
    if (done && harmonic_) {
      done_deferred_ = true;
      return false;
    }
    return done;
  }

 private:
  bool IsGrounded() const {
    return union_find_.CountRoots() == 1;
  }

  void UpdateUnionFind(const std::vector<Region>& fills) {
    for (const auto& fill : fills) {
      for (int x = fill.mini.x; x <= fill.maxi.x; ++x) {
        for (int y = fill.mini.y; y <= fill.maxi.y; ++y) {
          for (int z = fill.mini.z; z <= fill.maxi.z; ++z) {
            const Point p(x, y, z);
            union_find_.Add(p);
            union_find_.Connect(p, Point(x - 1, y, z));
            union_find_.Connect(p, Point(x + 1, y, z));
            union_find_.Connect(p, Point(x, y - 1, z));
            union_find_.Connect(p, Point(x, y + 1, z));
            union_find_.Connect(p, Point(x, y, z - 1));
            union_find_.Connect(p, Point(x, y, z + 1));
            if (y == 0) {
              union_find_.Connect(p, GROUND);
            }
          }
        }
      }
    }
  }

  const TaskPtr task_;
  const Point GROUND;
  UnionFind union_find_;
  bool harmonic_ = false;
  std::map<int, Command> holdback_commands_;
  bool holdback_done_;
  bool done_deferred_ = false;
};

TaskPtr MakeLazyFlipMiddlewareTask(TaskPtr task) {
  return MakeTask(new LazyFlipMiddlewareTask(std::move(task)));
}

}  // namespace

TaskPtr MakeLineAssemblerTask() {
  return MakeTask([](Task::Commander* cmd) -> TaskPtr {
    CHECK(MATRIX.IsEmpty()) << "precondition failed";
    CHECK_EQ(1, BOTS.size()) << "precondition failed";
    CHECK_EQ(Point(), BOT(0).position()) << "precondition failed";

    auto regions = ComputeRegions(cmd);

    return MakeSequenceTask(
        MakeFissionTask(),
        MakeScatterToRegionsTask(regions),
        MakeLazyFlipMiddlewareTask(MakeParallelLineAssembleTask(regions)),
        MakeFinishTask(),
        MakeCommandTask(0, Command::Halt()));
  });
}
