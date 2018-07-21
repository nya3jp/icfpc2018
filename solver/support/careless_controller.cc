#include "solver/support/careless_controller.h"

#include "glog/logging.h"

void CarelessController::Halt() {
  CHECK((current_ == Point{0, 0, 0}));
  writer_->Halt();
}

void CarelessController::Flip() {
  writer_->Flip();
}

void CarelessController::MoveDelta(const Delta &delta) {
  std::vector<LinearDelta> linears;
  if (delta.dx != 0) {
    linears.emplace_back(LinearDelta{Axis::X, delta.dx});
  }
  if (delta.dy != 0) {
    linears.emplace_back(LinearDelta{Axis::Y, delta.dy});
  }
  if (delta.dz != 0) {
    linears.emplace_back(LinearDelta{Axis::Z, delta.dz});
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

  current_ += delta;
}

void CarelessController::MoveTo(const Point& destination) {
  MoveDelta(destination - current_);
}

void CarelessController::FillBelow() {
  CHECK(current_.y > 0);
  writer_->Fill(Delta{0, -1, 0});
}
