#include "solver/tasks/plane_assembler.h"

#include <functional>

#include "glog/logging.h"

#include "solver/tasks/command.h"

#define FIELD (*cmd->field())
#define MATRIX (FIELD.matrix())
#define TARGET (FIELD.target())
#define BOTS (FIELD.bots())
#define BOT(bot_id) (BOTS.find(bot_id)->second)

namespace {

TaskPtr MakeGreedyMoveTask(int bot_id, Point destination) {
  return MakeTask([=](Task::Commander* cmd) -> bool {
    Delta delta = destination - BOT(bot_id).position();
    if (delta.IsZero()) {
      return true;
    }

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
        LinearDelta move{axis, std::max(std::min(component, cap), -cap)};
        if (cmd->Set(bot_id, Command::SMove(move))) {
          return linears.size() == 1 && std::abs(component) == cap;
        }
      }
    }
    return false;
  });
}

TaskPtr MakeLinearFissionTask(int bot_id) {
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
    return MakeSequenceTask(std::move(task), MakeLinearFissionTask(next_bot_id));
  });
}

TaskPtr MakeFissionTask() {
  return MakeSequenceTask(
      MakeCommandTask(0, Command::Fission(Delta(0, 0, 1), 19)),  // 0: [1, 40) -> 1
      MakeBarrierTask(
          MakeCommandTask(0, Command::Fission(Delta(0, 1, 0), 9)),   // 0: [21, 40) -> 21
          MakeCommandTask(1, Command::Fission(Delta(0, 1, 0), 9))),  // 1: [2, 21) -> 2
      MakeBarrierTask(
          MakeLinearFissionTask(0),
          MakeLinearFissionTask(1),
          MakeLinearFissionTask(21),
          MakeLinearFissionTask(2)));
}

TaskPtr MakeFusionTask() {
  //LOG(FATAL) << "NOT IMPLEMENTED";
  return MakeCommandTask(0, Command::Wait());
}

}  // namespace

TaskPtr MakePlaneAssemblerTask() {
  return MakeTask([](Task::Commander* cmd) -> TaskPtr {
    CHECK(MATRIX.IsEmpty()) << "precondition failed";
    CHECK_EQ(1, BOTS.size()) << "precondition failed";
    CHECK_EQ(Point(), BOT(0).position()) << "precondition failed";

    return MakeSequenceTask(
        MakeFissionTask(),
        MakeCommandTask(0, Command::Flip()),
        MakeCommandTask(0, Command::Flip()),
        MakeFusionTask()/*,
        MakeCommandTask(0, Command::Halt())*/);
  });
}
