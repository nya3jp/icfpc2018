#ifndef SOLVER_SUPPORT_CARELESS_CONTROLLER_H
#define SOLVER_SUPPORT_CARELESS_CONTROLLER_H

#include "solver/data/geometry.h"
#include "solver/io/trace_writer.h"

class CarelessController {
 public:
  CarelessController(int resolution, TraceWriter* writer)
      : resolution_(resolution), writer_(writer), current_(0, 0, 0) {}
  CarelessController(const CarelessController& other) = delete;

  void Halt();
  void Flip();
  void MoveDelta(const Delta &delta);
  void MoveTo(const Point& destination);
  void FillBelow();

  const Point& current() const { return current_; }

 private:

  void VerifyCurrent() const;

  const int resolution_;
  TraceWriter* const writer_;

  Point current_;
};

#endif // SOLVER_SUPPORT_CARELESS_CONTROLLER_H
