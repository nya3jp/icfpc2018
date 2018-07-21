#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "solver/geometry.h"
#include "solver/model.h"
#include "solver/trace_writer.h"

DEFINE_string(model, "", "Path to input model file");
DEFINE_string(trace, "", "Path to output trace file");

class Solver {
 public:
  Solver(Model* const model, TraceWriter* writer)
      : model_(model), writer_(writer), current_(0, 0, 0) {}
  Solver(const Solver& other) = delete;

  void Solve();

 private:
  void FreeMove(const Point& p);

  const Model* const model_;
  TraceWriter* const writer_;

  Point current_;
};

void Solver::Solve() {
  int n = model_->Resolution();

  writer_->Flip();

  FreeMove(Point{0, 1, 0});

  for (;;) {
    int min_x = n, min_z = n, max_x = -1, max_z = -1;
    for (int x = 0; x < n; ++x) {
      for (int z = 0; z < n; ++z) {
        //LOG(ERROR) << x << " " << z;
        if (model_->Get(x, current_.y - 1, z)) {
          min_x = std::min(min_x, x);
          min_z = std::min(min_z, z);
          max_x = std::max(max_x, x);
          max_z = std::max(max_z, z);
        }
      }
    }
    if (max_x == -1) {
      break;
    }

    int init_x = std::abs(min_x - current_.x) < std::abs(max_x - current_.x) ? min_x : max_x;
    int init_z = std::abs(min_z - current_.z) < std::abs(max_z - current_.z) ? min_z : max_z;
    int move_x = init_x == min_x ? 1 : -1;
    int move_z = init_z == min_z ? 1 : -1;
    int from_z = init_z;
    int to_z = init_z == min_z ? max_z : min_z;

    for (int x = init_x; min_x <= x && x <= max_x; x += move_x) {
      for (int z = from_z; min_z <= z && z <= max_z; z += move_z) {
        if (model_->Get(x, current_.y - 1, z)) {
          FreeMove(Point{x, current_.y, z});
          writer_->Fill(Delta{0, -1, 0});
        }
      }
      std::swap(from_z, to_z);
      move_z *= -1;
    }

    if (current_.y == n - 1) {
      break;
    }
    FreeMove(Point{current_.x, current_.y + 1, current_.z});
  }

  writer_->Flip();

  FreeMove(Point{0, current_.y, 0});
  FreeMove(Point{0, 0, 0});

  writer_->Halt();
}

void Solver::FreeMove(const Point& p) {
  Delta d = p - current_;

  std::vector<LinearDelta> linears;
  if (d.dx != 0) {
    linears.emplace_back(LinearDelta{Axis::X, d.dx});
  }
  if (d.dy != 0) {
    linears.emplace_back(LinearDelta{Axis::Y, d.dy});
  }
  if (d.dz != 0) {
    linears.emplace_back(LinearDelta{Axis::Z, d.dz});
  }

  if (linears.size() == 2 && std::max(std::abs(linears[0].delta), std::abs(linears[1].delta)) <= 5) {
    writer_->LMove(linears[0], linears[1]);
  } else {
    for (auto linear : linears) {
      while (linear.delta != 0) {
        LinearDelta move{linear.axis, std::max(std::min(linear.delta, 15), -15)};
        writer_->SMove(move);
        linear.delta -= move.delta;
      }
    }
  }

  current_ = p;
  //LOG(ERROR) << p.x << " " << p.y << " " << p.z;
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  if (FLAGS_model.empty() || FLAGS_trace.empty()) {
    LOG(ERROR) << "Usage: " << argv[0] << " --model=LA000.mdl --trace=out.nbt";
    return 1;
  }

  std::unique_ptr<Model> model;
  {
    std::ifstream fin(FLAGS_model.c_str());
    model = Model::FromStream(fin);
  }

  std::ofstream fout(FLAGS_trace.c_str());
  TraceWriter writer(fout);

  Solver solver(model.get(), &writer);
  solver.Solve();

  return 0;
}
