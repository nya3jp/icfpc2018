#include "solver/io/trace_writer.h"

void TraceWriter::Halt() {
  WriteByte(0b11111111);
}

void TraceWriter::Wait() {
  WriteByte(0b11111110);
}

void TraceWriter::Flip() {
  WriteByte(0b11111101);
}

void TraceWriter::SMove(const LinearDelta& lld) {
  CHECK(lld.IsLong()) << lld;
  WriteByte(0b00000100 | ((static_cast<int>(lld.axis) + 1) << 4));
  WriteByte(lld.delta + LONG_LEN);
  //LOG(ERROR) << "SMove " << (int)lld.axis << " " << lld.delta;
}

void TraceWriter::LMove(const LinearDelta& sld1, const LinearDelta& sld2) {
  CHECK(sld1.IsShort()) << sld1;
  CHECK(sld2.IsShort()) << sld2;
  WriteByte(0b00001100 | ((static_cast<int>(sld1.axis) + 1) << 4) | ((static_cast<int>(sld2.axis) + 1) << 6));
  WriteByte((sld1.delta + SHORT_LEN) | ((sld2.delta + SHORT_LEN) << 4));
  //LOG(ERROR) << "LMove " << (int)sld1.axis << " " << sld1.delta << " " << (int)sld2.axis << " " << sld2.delta;
}

void TraceWriter::Fill(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000011 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  //LOG(ERROR) << "Fill";
}

void TraceWriter::Void(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000010 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  //LOG(ERROR) << "Void";
}

void TraceWriter::Fission(const Delta& nd, int nchildren) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000101 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  WriteByte(nchildren);
}

void TraceWriter::FusionP(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000111 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
}

void TraceWriter::FusionS(const Delta& nd) {
  CHECK(nd.IsNear()) << nd;
  WriteByte(0b00000110 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
}

void TraceWriter::Gfill(const Delta& nd, const Delta& fd) {
  CHECK(nd.IsNear()) << nd;
  CHECK(fd.IsFar()) << fd;
  WriteByte(0b00000001 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  WriteByte(fd.dx + 30);
  WriteByte(fd.dy + 30);
  WriteByte(fd.dz + 30);
  //LOG(ERROR) << "Gfill";
}

void TraceWriter::Gvoid(const Delta& nd, const Delta& fd) {
  CHECK(nd.IsNear()) << nd;
  CHECK(fd.IsFar()) << fd;
  WriteByte(0b00000000 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  WriteByte(fd.dx + 30);
  WriteByte(fd.dy + 30);
  WriteByte(fd.dz + 30);
  //LOG(ERROR) << "Gvoid";
}

void TraceWriter::WriteByte(uint8_t b) {
  os_.write(reinterpret_cast<char*>(&b), 1);
}
