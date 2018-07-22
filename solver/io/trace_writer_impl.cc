#include "solver/io/trace_writer.h"

#include "glog/logging.h"

// To enable logging, specify: --vmodule=trace_writer=1

void TraceWriter::Halt() {
  WriteByte(0b11111111);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Halt";
  }
}

void TraceWriter::Wait() {
  WriteByte(0b11111110);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Wait";
  }
}

void TraceWriter::Flip() {
  WriteByte(0b11111101);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Flip";
  }
}

void TraceWriter::SMove(const LinearDelta& lld) {
  CHECK(lld.IsLong()) << lld;
  WriteByte(0b00000100 | ((static_cast<int>(lld.axis) + 1) << 4));
  WriteByte(lld.delta + LONG_LEN);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "SMove " << lld;
  }
}

void TraceWriter::LMove(const LinearDelta& sld1, const LinearDelta& sld2) {
  CHECK(sld1.IsShort()) << sld1;
  CHECK(sld2.IsShort()) << sld2;
  WriteByte(0b00001100 | ((static_cast<int>(sld1.axis) + 1) << 4) | ((static_cast<int>(sld2.axis) + 1) << 6));
  WriteByte((sld1.delta + SHORT_LEN) | ((sld2.delta + SHORT_LEN) << 4));
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "LMove " << sld1 << " " << sld2;
  }
}

void TraceWriter::Fill(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000011 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Fill " << nd;
  }
}

void TraceWriter::Void(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000010 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Void" << nd;
  }
}

void TraceWriter::Fission(const Delta& nd, int nchildren) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000101 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  WriteByte(nchildren);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Fission " << nd << " " << nchildren;
  }
}

void TraceWriter::FusionP(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000111 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "FusionP " << nd;
  }
}

void TraceWriter::FusionS(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000110 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "FusionS " << nd;
  }
}

void TraceWriter::Gfill(const Delta& nd, const Delta& fd) {
  CHECK(nd.IsNear()) << nd;
  CHECK(fd.IsFar()) << fd;
  WriteByte(0b00000001 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  WriteByte(fd.dx + 30);
  WriteByte(fd.dy + 30);
  WriteByte(fd.dz + 30);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Gfill " << nd << " " << fd;
  }
}

void TraceWriter::Gvoid(const Delta& nd, const Delta& fd) {
  CHECK(nd.IsNear()) << nd;
  CHECK(fd.IsFar()) << fd;
  WriteByte(0b00000000 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  WriteByte(fd.dx + 30);
  WriteByte(fd.dy + 30);
  WriteByte(fd.dz + 30);
  if (VLOG_IS_ON(1)) {  // See comment at the top
    VLOG(1) << "Gvoid " << nd << " " << fd;
  }
}

void TraceWriter::Command(const struct Command& command) {
  switch (command.type) {
    case Command::HALT:
      Halt();
      break;
    case Command::WAIT:
      Wait();
      break;
    case Command::FLIP:
      Flip();
      break;
    case Command::LMOVE:
      LMove(command.ld1, command.ld2);
      break;
    case Command::SMOVE:
      SMove(command.ld1);
      break;
    case Command::FILL:
      Fill(command.nd);
      break;
    case Command::VOID:
      Fill(command.nd);
      break;
    case Command::GFILL:
      Gfill(command.nd, command.fd);
      break;
    case Command::GVOID:
      Gvoid(command.nd, command.fd);
      break;
    case Command::FISSION:
      Fission(command.nd, command.arg);
      break;
    case Command::FUSION_MASTER:
      FusionP(command.nd);
      break;
    case Command::FUSION_SLAVE:
      FusionS(command.nd);
      break;
    default:
      LOG(FATAL) << "Malformed command type";
  }
}

void TraceWriter::WriteByte(uint8_t b) {
  os_.write(reinterpret_cast<char*>(&b), 1);
}
