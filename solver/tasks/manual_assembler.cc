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

    CHECK(++stuck < 100) << "Bot " << bot_id << " stuck: " << position << " -> " << destination;

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

TaskPtr MakeRegionFillVoidTask(bool fill, Region region) {
  return MakeTask([=](Task::Commander* cmd) -> TaskPtr {
    auto FindEmptyNeighbor = [&](Point p) -> Point {
      for (auto axis : {Axis::Y, Axis::X, Axis::Z}) {
        for (int d : {1, -1}) {
          Point adj = p + LinearDelta(axis, d).ToDelta();
          if (!region.Contains(adj) && MATRIX.IsMovable(Region::FromPoint(adj))) {
            return adj;
          }
        }
      }
      LOG(FATAL) << "No empty neighbor";
      return Point();
    };

    int dim =
        (region.mini.x != region.maxi.x ? 1 : 0) +
        (region.mini.y != region.maxi.y ? 1 : 0) +
        (region.mini.z != region.maxi.z ? 1 : 0);
    if (dim == 0) {
      Point p = region.mini;
      Point adj = FindEmptyNeighbor(p);
      Command command = Command::Void(p - adj);
      if (fill) {
        command.type = Command::FILL;
      }
      std::vector<TaskPtr> tasks;
      for (int bot_id = 1; bot_id < 8; ++bot_id) {
        const Point& t = BOT(bot_id).position();
        if (t == p || t == adj) {
          tasks.emplace_back(MakeGreedyMoveTask(bot_id, Point(bot_id, 0, 0)));
        }
      }
      tasks.emplace_back(MakeSequenceTask(
          MakeGreedyMoveTask(0, adj),
          MakeCommandTask(0, command)));
      return MakeBarrierTask(std::move(tasks));
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

    std::vector<TaskPtr> move_tasks;
    std::vector<TaskPtr> group_tasks;
    std::set<Point> adjs;
    int bot_id = 0;
    for (const auto& diagonal : diagonals) {
      Point adj = FindEmptyNeighbor(diagonal.first);
      move_tasks.emplace_back(MakeGreedyMoveTask(bot_id, adj));
      Command command = Command::GVoid(diagonal.first - adj, diagonal.second - diagonal.first);
      if (fill) {
        command.type = Command::GFILL;
      }
      group_tasks.emplace_back(MakeCommandTask(bot_id, command));
      adjs.insert(adj);
      ++bot_id;
    }
    std::vector<TaskPtr> tasks;
    for (; bot_id < 8; ++bot_id) {
      Point t = BOT(bot_id).position();
      if (region.Contains(t) || adjs.count(t) > 0) {
        tasks.emplace_back(MakeGreedyMoveTask(bot_id, Point(bot_id, 0, 0)));
      }
    }
    tasks.emplace_back(MakeSequenceTask(MakeBarrierTask(std::move(move_tasks)), MakeBarrierTask(std::move(group_tasks))));
    return MakeBarrierTask(std::move(tasks));
  });
}

}  // namespace

TaskPtr Fill(Region region) {
  return MakeRegionFillVoidTask(true, region);
}

TaskPtr Void(Region region) {
  return MakeRegionFillVoidTask(false, region);
}

TaskPtr MakeManualAssemblerTask(TaskPtr main_task) {
  return MakeSequenceTask(
      MakeFissionTask(),
      std::move(main_task),
      MakeFinishTask(),
      MakeCommandTask(0, Command::Halt()));
}
