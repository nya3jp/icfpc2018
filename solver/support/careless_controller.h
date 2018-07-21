#ifndef SOLVER_SUPPORT_CARELESS_CONTROLLER_H
#define SOLVER_SUPPORT_CARELESS_CONTROLLER_H

#include "solver/data/geometry.h"
#include "solver/io/trace_writer.h"

class CarelessController {
 public:
  explicit CarelessController(TraceWriter* writer)
      : writer_(writer), current_(0, 0, 0) {}
  CarelessController(const CarelessController& other) = delete;

  void Halt();
  void Flip();
  void MoveTo(const Point &destination);
  void FillBelow();

  const Point& current() const { return current_; }

 private:
  TraceWriter* const writer_;

  Point current_;
};

#endif // SOLVER_SUPPORT_CARELESS_CONTROLLER_H
