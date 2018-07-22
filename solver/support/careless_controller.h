#ifndef SOLVER_SUPPORT_CARELESS_CONTROLLER_H
#define SOLVER_SUPPORT_CARELESS_CONTROLLER_H

#include <memory>

#include "solver/data/geometry.h"
#include "solver/io/trace_writer.h"

class CarelessController {
 public:
  CarelessController(int resolution, TraceWriter* writer)
      : CarelessController(resolution, writer, Point(), 0, (static_cast<uint64_t>(1) << 40) - 2) {}
  CarelessController(int resolution, TraceWriter* writer, Point current, int bid, uint64_t seeds)
      : resolution_(resolution), writer_(writer), current_(current), bid_(bid), seeds_(seeds) {}
  CarelessController(const CarelessController& other) = delete;

  void Halt();
  void Wait();
  void Flip();
  void MoveDelta(const Delta &delta);
  void MoveTo(const Point& destination);
  void FillBelow();
  CarelessController* Fission(const Delta& delta, int nchildren);
  void FusionP(const Delta& delta);
  void FusionS(const Delta& delta);
  void Gfill(const Delta& nd, const Delta& fd);
  void Gvoid(const Delta& nd, const Delta& fd);

  bool operator<(const CarelessController& o) const;

  const Point& current() const { return current_; }

 private:
  void VerifyCurrent() const;

  const int resolution_;
  TraceWriter* const writer_;
  Point current_;
  const int bid_;
  uint64_t seeds_;
};

#endif // SOLVER_SUPPORT_CARELESS_CONTROLLER_H
