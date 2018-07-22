#ifndef SOLVER_IO_TRACE_WRITER_IMPL_H
#define SOLVER_IO_TRACE_WRITER_IMPL_H

#include <stdint.h>

#include <iostream>

#include "solver/io/trace_writer.h"

class TraceWriterImpl : public TraceWriter {
 public:
  explicit TraceWriterImpl(std::ostream& os) : os_(os) {};
  TraceWriterImpl(const TraceWriterImpl& other) = delete;

  void Command(const struct Command& command);

 private:
  void Halt();
  void Wait();
  void Flip();
  void SMove(const LinearDelta& lld);
  void LMove(const LinearDelta& sld1, const LinearDelta& sld2);
  void Fill(const Delta& nd);
  void Void(const Delta& nd);
  void Fission(const Delta& nd, int nchildren);
  void FusionP(const Delta& nd);
  void FusionS(const Delta& nd);
  void Gfill(const Delta& nd, const Delta& fd);
  void Gvoid(const Delta& nd, const Delta& fd);

  void WriteByte(uint8_t b);

  std::ostream& os_;
};

#endif // SOLVER_IO_TRACE_WRITER_IMPL_H
