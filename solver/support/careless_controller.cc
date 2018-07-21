#include "solver/support/careless_controller.h"

#include "glog/logging.h"

void CarelessController::Halt() {
  CHECK(current_.IsOrigin());
  writer_->Halt();
}

void CarelessController::Wait() {
  writer_->Wait();
}

void CarelessController::Flip() {
  writer_->Flip();
}

void CarelessController::MoveDelta(const Delta &delta) {
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
    writer_->LMove(linears[0], linears[1]);
  } else {
    for (auto linear : linears) {
      while (linear.delta != 0) {
        LinearDelta move{
            linear.axis,
            std::max(std::min(linear.delta, LONG_LEN), -LONG_LEN)};
        writer_->SMove(move);
        linear.delta -= move.delta;
      }
    }
  }

  current_ += delta;

  VerifyCurrent();
}

void CarelessController::MoveTo(const Point& destination) {
  MoveDelta(destination - current_);
}

void CarelessController::FillBelow() {
  CHECK(current_.y > 0);
  writer_->Fill(Delta{0, -1, 0});
}

void CarelessController::VerifyCurrent() const {
  CHECK(0 <= current_.x && current_.x < resolution_ &&
        0 <= current_.y && current_.y < resolution_ &&
        0 <= current_.z && current_.z < resolution_)
      << "(" << current_.x << ", " << current_.y << ", " << current_.z << "); resolution=" << resolution_;
}

std::unique_ptr<CarelessController> CarelessController::Fission(const Delta& delta, int nchildren) {
  Point newcurrent = current_ + delta;
  CHECK(__builtin_popcountl(seeds_) >= nchildren + 1);
  writer_->Fission(delta, nchildren);

  uint64_t newbid = __builtin_ctzl(seeds_);
  seeds_ ^= static_cast<uint64_t>(1) << newbid;
  uint64_t newseeds = 0;
  for (int i = 0; i < nchildren; ++i) {
    newseeds ^= static_cast<uint64_t>(1) << __builtin_ctzl(seeds_);
  }
  seeds_ ^= newseeds;
  return std::unique_ptr<CarelessController>(new CarelessController(resolution_, writer_, newcurrent, newbid, newseeds));
}

void CarelessController::FusionP(const Delta& delta) {
  writer_->FusionP(delta);
}

void CarelessController::FusionS(const Delta& delta) {
  writer_->FusionS(delta);
}

void CarelessController::Gfill(const Delta& nd, const Delta& fd) {
  writer_->Gfill(nd, fd);
}

void CarelessController::Gvoid(const Delta& nd, const Delta& fd) {
  writer_->Gvoid(nd, fd);
}

bool CarelessController::operator<(const CarelessController& o) const {
  return bid_ < o.bid_;
}
