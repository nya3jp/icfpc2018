#include "solver/tasks/manual_assembler.h"

#include "glog/logging.h"

#include "solver/support/union_find.h"
#include "solver/tasks/command.h"

#define FIELD (*cmd->field())
#define MATRIX (FIELD.matrix())
#define TARGET (FIELD.target())
#define BOTS (FIELD.bots())
#define BOT(bot_id) (BOTS.find(bot_id)->second)

namespace {

inline int CapAbs(int delta, int cap) {
  return std::max(std::min(delta, cap), -cap);
}

inline int Sign(int delta) {
  return delta == 0 ? 0 : delta > 0 ? 1 : -1;
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

TaskPtr MakeFissionTask() {
  return MakeSequenceTask(
    MakeCommandTask(0, Command::Fission(Delta(1, 0, 0), 3)),  // 0[1, 40) -> 0[5, 40), 1[2, 5)
    MakeBarrierTask(
        MakeCommandTask(0, Command::Fission(Delta(0, 0, 1), 1)),   // 0[5, 40) -> 0[8, 40), 5[6, 7)
        MakeCommandTask(1, Command::Fission(Delta(0, 0, 1), 1))),  // 1[2, 5) -> 1[4, 5), 2[3, 4)
    MakeBarrierTask(
        MakeCommandTask(0, Command::Fission(Delta(0, 1, 0), 0)),
        MakeCommandTask(1, Command::Fission(Delta(0, 1, 0), 0)),
        MakeCommandTask(5, Command::Fission(Delta(0, 1, 0), 0)),
        MakeCommandTask(2, Command::Fission(Delta(0, 1, 0), 0))));
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

    const Point meeting_point(0, 0, 0);
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
        MakeRoughFusionTask(),
        MakeGoOriginTask());
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

TaskPtr MakePointFillTask(Point p) {
  return MakeSequenceTask(
      MakeGreedyMoveTask(0, p + Delta(0, 1, 0)),
      MakeCommandTask(0, Command::Fill(Delta(0, -1, 0))));
}

TaskPtr MakeGroupFillTask(Region region, std::vector<std::pair<Point, Point>> diagonals) {
  std::vector<TaskPtr> move_tasks;
  std::vector<TaskPtr> gfill_tasks;
  int bot_id = 0;
  for (const auto& diagonal : diagonals) {
    Delta adj(diagonal.first.x == region.mini.x ? -1 : 1, 0, 0);
    move_tasks.emplace_back(MakeGreedyMoveTask(bot_id, diagonal.first + adj));
    gfill_tasks.emplace_back(MakeCommandTask(bot_id, Command::GFill(Delta() - adj, diagonal.second - diagonal.first)));
    ++bot_id;
  }
  return MakeSequenceTask(MakeBarrierTask(std::move(move_tasks)), MakeBarrierTask(std::move(gfill_tasks)));
}

}  // namespace

TaskPtr Fill(Region region) {
  int dim =
      (region.mini.x != region.maxi.x ? 1 : 0) +
      (region.mini.y != region.maxi.y ? 1 : 0) +
      (region.mini.z != region.maxi.z ? 1 : 0);
  if (dim == 0) {
    return MakePointFillTask(region.mini);
  }

  std::set<std::pair<Point, Point>> diagonals;
  for (int xa : {region.mini.x, region.maxi.x}) {
    int xb = (xa == region.mini.x ? region.maxi.x : region.mini.x);
    for (int ya : {region.mini.y, region.maxi.y}) {
      int yb = (ya == region.mini.y ? region.maxi.y : region.mini.y);
      for (int za : {region.mini.z, region.maxi.z}) {
        int zb = (za == region.mini.z ? region.maxi.z : region.mini.z);
        diagonals.insert(std::make_pair(Point(xa, ya, za), Point(xb, yb, zb)));
      }
    }
  }
  CHECK_EQ(diagonals.size(), 1 << dim);

  return MakeGroupFillTask(region, std::vector<std::pair<Point, Point>>(diagonals.begin(), diagonals.end()));
}

TaskPtr MakeManualAssemblerTask(TaskPtr main_task) {
  return MakeSequenceTask(
      MakeFissionTask(),
      MakeLazyFlipMiddlewareTask(std::move(main_task)),
      MakeFinishTask(),
      MakeCommandTask(0, Command::Halt()));
}
