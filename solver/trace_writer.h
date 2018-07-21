#ifndef SOLVER_TRACE_WRITER_H
#define SOLVER_TRACE_WRITER_H

#include <stdint.h>

#include <iostream>

#include "solver/geometry.h"

class TraceWriter {
 public:
  explicit TraceWriter(std::ostream& os) : os_(os) {};
  TraceWriter(const TraceWriter& other) = delete;

  void Halt();
  void Flip();
  void SMove(const LinearDelta& lld);
  void LMove(const LinearDelta& sld1, const LinearDelta& sld2);
  void Fill(const Delta& nd);

 private:
  void WriteByte(uint8_t b);

  std::ostream& os_;
};

#endif // SOLVER_TRACE_WRITER_H
