#include "solver/support/careless_controller.h"

#include "glog/logging.h"

void CarelessController::Halt() {
  CHECK(current_.IsOrigin());
  writer_->Command(Command::Halt());
}

void CarelessController::Wait() {
  writer_->Command(Command::Wait());
}

void CarelessController::Flip() {
  writer_->Command(Command::Flip());
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
    writer_->Command(Command::LMove(linears[0], linears[1]));
  } else {
    for (auto linear : linears) {
      while (linear.delta != 0) {
        LinearDelta move{
            linear.axis,
            std::max(std::min(linear.delta, LONG_LEN), -LONG_LEN)};
        writer_->Command(Command::SMove(move));
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
  writer_->Command(Command::Fill(Delta{0, -1, 0}));
}

void CarelessController::VerifyCurrent() const {
  CHECK(0 <= current_.x && current_.x < resolution_ &&
        0 <= current_.y && current_.y < resolution_ &&
        0 <= current_.z && current_.z < resolution_)
      << current_ << "; resolution=" << resolution_;
}

CarelessController* CarelessController::Fission(const Delta& delta, int nchildren) {
  CHECK(delta.IsNear());
  Point newcurrent = current_ + delta;
  CHECK(__builtin_popcountl(seeds_) >= nchildren + 1) << bid_ << " " << seeds_ << " " << nchildren;
  writer_->Command(Command::Fission(delta, nchildren));

  uint64_t newbid = __builtin_ctzl(seeds_);
  seeds_ ^= static_cast<uint64_t>(1) << newbid;
  uint64_t newseeds = 0;
  for (int i = 0; i < nchildren; ++i) {
    uint64_t bit = static_cast<uint64_t>(1) << __builtin_ctzl(seeds_);
    newseeds ^= bit;
    seeds_ ^= bit;
  }
  return new CarelessController(resolution_, writer_, newcurrent, newbid, newseeds);
}

void CarelessController::FusionP(const Delta& delta) {
  CHECK(delta.IsNear()) << delta;
  writer_->Command(Command::FusionP(delta));
}

void CarelessController::FusionS(const Delta& delta) {
  CHECK(delta.IsNear()) << delta;
  writer_->Command(Command::FusionS(delta));
}

void CarelessController::Gfill(const Delta& nd, const Delta& fd) {
  CHECK(nd.IsNear()) << nd;
  CHECK(fd.IsFar()) << fd;
  writer_->Command(Command::GFill(nd, fd));
}

void CarelessController::Gvoid(const Delta& nd, const Delta& fd) {
  CHECK(nd.IsNear()) << nd;
  CHECK(fd.IsFar()) << fd;
  writer_->Command(Command::GVoid(nd, fd));
}

bool CarelessController::operator<(const CarelessController& o) const {
  return bid_ < o.bid_;
}
