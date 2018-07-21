#include "solver/support/careless_controller.h"

#include "glog/logging.h"

void CarelessController::Halt() {
  CHECK((current_ == Point{0, 0, 0}));
  writer_->Halt();
}

void CarelessController::Flip() {
  writer_->Flip();
}

void CarelessController::MoveTo(const Point &destination) {
  Delta d = destination - current_;

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

  current_ = destination;
}

void CarelessController::FillBelow() {
  CHECK(current_.y > 0);
  writer_->Fill(Delta{0, -1, 0});
}
