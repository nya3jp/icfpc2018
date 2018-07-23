#include "solver/impls/bbgvoid_task.h"

#include "glog/logging.h"

#include "solver/data/state.h"
#include "solver/support/task_executor.h"
#include "solver/support/task.h"

namespace {

TaskPtr MakeFreeMoveTask(int bot_id, Point destination) {
  return MakeTask([=](Task::Commander* commander) -> bool {
    const Point position = commander->field()->bots().find(bot_id)->second.position();
    const Delta delta = destination - position;
    CHECK(position != destination);

    if (delta.dy != 0 && std::abs(delta.dy) > SHORT_LEN) {
      auto move = LinearDelta(Axis::Y, std::max(std::min(delta.dy, LONG_LEN), -LONG_LEN));
      bool success = commander->Set(bot_id, Command::SMove(move));
      CHECK(success);
      return position + move.ToDelta() == destination;
    }
    if (delta.dx != 0 && std::abs(delta.dx) > SHORT_LEN) {
      auto move = LinearDelta(Axis::X, std::max(std::min(delta.dx, LONG_LEN), -LONG_LEN));
      bool success = commander->Set(bot_id, Command::SMove(move));
      CHECK(success);
      return position + move.ToDelta() == destination;
    }
    if (delta.dz != 0 && std::abs(delta.dz) > SHORT_LEN) {
      auto move = LinearDelta(Axis::Z, std::max(std::min(delta.dz, LONG_LEN), -LONG_LEN));
      bool success = commander->Set(bot_id, Command::SMove(move));
      CHECK(success);
      return position + move.ToDelta() == destination;
    }

    std::vector<LinearDelta> linears;
    if (delta.dy != 0) {
      linears.emplace_back(Axis::Y, delta.dy);
    }
    if (delta.dx != 0) {
      linears.emplace_back(Axis::X, delta.dx);
    }
    if (delta.dz != 0) {
      linears.emplace_back(Axis::Z, delta.dz);
    }

    if (linears.size() == 1) {
      bool success = commander->Set(bot_id, Command::SMove(linears[0]));
      CHECK(success);
    } else {
      bool success = commander->Set(bot_id, Command::LMove(linears[0], linears[1]));
      CHECK(success);
    }
    return linears.size() < 3;
  });
}

TaskPtr MakeInitMoveTask(Region& bb_) {
  return MakeTask([bb_](Task::Commander *commander) -> TaskPtr {
    const auto& bots = commander->field()->bots();
    CHECK_EQ(bots.size(), 1);

    auto dest = Point(bb_.mini.x - 1, bb_.mini.y, bb_.mini.z - 1);

    for (const auto& pair : bots) {
      int bot_id = pair.first;
      return MakeFreeMoveTask(bot_id, dest);
    }
    CHECK(false);
  });
}

// Schedule like this:
//
// 0  1  2  3  4  5  6  7
// Fx
// Fz Mx
// Fy Fz Mz
//    Fy Fy Mz My
//          Fy    My My
//                      My
TaskPtr MakeFissionTask(Region& bb_) {
  return MakeTask([bb_](Task::Commander *commander) -> bool {
    const auto& bots = commander->field()->bots();
    std::vector<int> bot_ids(8, -1);

    for (const auto& pair : bots) {
      int bot_id = pair.first;
      const auto& bot = pair.second;
      int i = (bot.position().y != bb_.mini.y ? 4 : 0) +
              (bot.position().z != bb_.mini.z - 1 ? 2 : 0) +
              (bot.position().x != bb_.mini.x - 1 ? 1 : 0);
      bot_ids[i] = bot_id;
    }
    bool done = bots.size() == 8;
    for (int i = 0; i < 8; ++i) {
      int bot_id = bot_ids[i];
      if (bot_id < 0) continue;
      const auto& bot = bots.at(bot_id);
      auto dest = Point(i & 1 ? bb_.maxi.x + 1 : bb_.mini.x - 1,
                        i & 4 ? bb_.maxi.y : bb_.mini.y,
                        i & 2 ? bb_.maxi.z + 1 : bb_.mini.z - 1);
      if (bot.position() != dest) {
        Axis axis = i < 2 ? Axis::X : i < 4 ? Axis::Z : Axis::Y;
        int delta = i < 2 ? dest.x - bot.position().x :
                    i < 4 ? dest.z - bot.position().z :
                            dest.y - bot.position().y;
        bool success = commander->Set(bot_id, Command::SMove(LinearDelta(axis, std::min(delta, LONG_LEN))));
        CHECK(success);
        if (delta > LONG_LEN) {
          done = false;
        }
      } else {
        if (i < 1 && bot_ids[i + 1] < 0) {
          bool success = commander->Set(bot_id, Command::Fission(Delta(1, 0, 0), 3));
          CHECK(success);
        } else if (i < 2 && bot_ids[i + 2] < 0) {
          bool success = commander->Set(bot_id, Command::Fission(Delta(0, 0, 1), 1));
          CHECK(success);
        } else if (i < 4 && bot_ids[i + 4] < 0) {
          bool success = commander->Set(bot_id, Command::Fission(Delta(0, 1, 0), 0));
          CHECK(success);
        }
      }
    }
    return done;
  });
}

TaskPtr MakeGvoidTask(Region& bb_) {
  return MakeTask([bb_](Task::Commander *commander) -> bool {
    const auto& bots = commander->field()->bots();

    int fx = bb_.maxi.x - bb_.mini.x;
    int fy = bb_.maxi.y - bb_.mini.y;
    int fz = bb_.maxi.z - bb_.mini.z;
    for (const auto& pair : bots) {
      int bot_id = pair.first;
      const auto& bot = pair.second;
      int dy = bot.position().y != bb_.mini.y ? -1 : 1;
      int dz = bot.position().z != bb_.mini.z - 1 ? -1 : 1;
      int dx = bot.position().x != bb_.mini.x - 1 ? -1 : 1;
      bool success = commander->Set(bot_id, Command::GVoid(Delta{dx, 0, dz}, Delta{dx * fx, dy * fy, dz * fz}));
      CHECK(success);
    }
    return true;
  });
}

TaskPtr MakeFusionTask(Region& bb_) {
  return MakeTask([bb_](Task::Commander *commander) -> bool {
    const auto& bots = commander->field()->bots();
    std::vector<int> bot_ids(8, -1);
    std::vector<bool> fusionable(8);

    for (const auto& pair : bots) {
      int bot_id = pair.first;
      const auto& bot = pair.second;
      int i = (bot.position().y != bb_.mini.y ? 4 : 0) +
              (bot.position().z != bb_.mini.z - 1 ? 2 : 0) +
              (bot.position().x != bb_.mini.x - 1 ? 1 : 0);
      auto dest = Point(i & 6 ? i & 1 ? bb_.maxi.x + 1 : bb_.mini.x - 1 : bb_.mini.x - 1 + (i & 1 ? 1 : 0),
                        bb_.mini.y + (i & 4 ? 1 : 0),
                        i & 4 ? i & 2 ? bb_.maxi.z + 1 : bb_.mini.z - 1 : bb_.mini.z - 1 + (i & 2 ? 1 : 0));
      bot_ids[i] = bot_id;
      fusionable[i] = bot.position() == dest;
    }
    bool done = false;
    for (int i = 0; i < 8; ++i) {
      int bot_id = bot_ids[i];
      if (bot_id < 0) continue;
      const auto& bot = bots.at(bot_id);
      if (i < 2 && fusionable[i | 1]) {
        if (i & 1) {
          bool success = commander->Set(bot_id, Command::FusionS(Delta{-1, 0, 0}));
          CHECK(success);
        } else {
          bool success = commander->Set(bot_id, Command::FusionP(Delta{1, 0, 0}));
          CHECK(success);
        }
        done = true;
      } else if (i < 4 && fusionable[i | 2]) {
        if (i & 2) {
          bool success = commander->Set(bot_id, Command::FusionS(Delta{0, 0, -1}));
          CHECK(success);
        } else {
          bool success = commander->Set(bot_id, Command::FusionP(Delta{0, 0, 1}));
          CHECK(success);
        }
      } else if (fusionable[i | 4]) {
        if (i & 4) {
          bool success = commander->Set(bot_id, Command::FusionS(Delta{0, -1, 0}));
          CHECK(success);
        } else {
          bool success = commander->Set(bot_id, Command::FusionP(Delta{0, 1, 0}));
          CHECK(success);
        }
      } else if (i >= 4 ||
                 (i >= 2 && bot_ids[i | 4] < 0) ||
                 (i >= 1 && bot_ids[i | 2] < 0)) {
        auto dest = Point(i & 6 ? i & 1 ? bb_.maxi.x + 1 : bb_.mini.x - 1 : bb_.mini.x - 1 + (i & 1 ? 1 : 0),
                          bb_.mini.y + (i & 4 ? 1 : 0),
                          i & 4 ? i & 2 ? bb_.maxi.z + 1 : bb_.mini.z - 1 : bb_.mini.z - 1 + (i & 2 ? 1 : 0));
        Axis axis = i < 2 ? Axis::X : i < 4 ? Axis::Z : Axis::Y;
        int delta = i < 2 ? dest.x - bot.position().x :
                    i < 4 ? dest.z - bot.position().z :
                            dest.y - bot.position().y;
        bool success = commander->Set(bot_id, Command::SMove(LinearDelta(axis, std::max(delta, -LONG_LEN))));
        CHECK(success);
      }
    }
    return done;
  });
}

TaskPtr MakeLastMoveTask() {
  return MakeTask([](Task::Commander *commander) -> TaskPtr {
    const auto& bots = commander->field()->bots();
    CHECK_EQ(bots.size(), 1);

    auto dest = Point(0, 0, 0);

    for (const auto& pair : bots) {
      int bot_id = pair.first;
      return MakeFreeMoveTask(bot_id, dest);
    }
    CHECK(false);
  });
}

TaskPtr MakeHaltTask() {
  return MakeTask([](Task::Commander* commander) -> bool {
    const auto& bots = commander->field()->bots();
    CHECK_EQ(bots.size(), 1);

    for (const auto& pair : bots) {
      int bot_id = pair.first;
      commander->Set(bot_id, Command::Halt());
      return true;
    }
    CHECK(false);
  });
}

TaskPtr MakeMasterTask(Region& bb_) {
  if (bb_.mini.x != 1 || bb_.mini.z != 1){
    return MakeSequenceTask(
        MakeInitMoveTask(bb_),
        MakeFissionTask(bb_),
        MakeGvoidTask(bb_),
        MakeFusionTask(bb_),
        MakeLastMoveTask(),
        MakeHaltTask());
  } else {
    return MakeSequenceTask(
        MakeFissionTask(bb_),
        MakeGvoidTask(bb_),
        MakeFusionTask(bb_),
        MakeHaltTask());
  }
}

}  // namespace

BBGvoidTaskSolver::BBGvoidTaskSolver(
    const Matrix* source, const Matrix* target, TraceWriter* writer)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer) {
    CHECK(target->IsEmpty()) << "target should be empty for BBGvoid solver";
}

void BBGvoidTaskSolver::Solve() {
  Region bb = CalculateBB();
  TaskExecutor executor(MakeMasterTask(bb));
  executor.Run(&field_, writer_);
  CHECK(field_.IsHalted());
}

Region BBGvoidTaskSolver::CalculateBB() {
  int resolution = field_.matrix().Resolution();
  int min_x = resolution, min_y = resolution, min_z = resolution;
  int max_x = -1, max_y = -1, max_z = -1;
  for (int x = 0; x < resolution; ++x) {
    for (int y = 0; y < resolution; ++y) {
      for (int z = 0; z < resolution; ++z) {
        if (field_.matrix().Get(x, y, z)) {
          min_x = std::min(min_x, x);
          min_y = std::min(min_y, y);
          min_z = std::min(min_z, z);
          max_x = std::max(max_x, x);
          max_y = std::max(max_y, y);
          max_z = std::max(max_z, z);
        }
      }
    }
  }
  // optimization: cut master's moving cost (ticks)
  min_x = 1, min_y = 0, min_z = 1;
  CHECK(max_x != min_x && max_y != min_y && max_z != min_z) << "too thin, cannot solve by BBGvoid";
  CHECK(max_x - min_x <= FAR_LEN && max_y - min_y <= FAR_LEN && max_z - min_z <= FAR_LEN) << "too large, cannot solve by BBGvoid";
  return Region(Point(min_x, min_y, min_z), Point(max_x, max_y, max_z));
}
