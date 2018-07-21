#include "solver/io/trace_writer.h"

void TraceWriter::Halt() {
  WriteByte(0b11111111);
}

void TraceWriter::Flip() {
  WriteByte(0b11111101);
}

void TraceWriter::SMove(const LinearDelta& lld) {
  CHECK(-15 <= lld.delta && lld.delta <= 15);
  WriteByte(0b00000100 | ((static_cast<int>(lld.axis) + 1) << 4));
  WriteByte(lld.delta + 15);
  //LOG(ERROR) << "SMove " << (int)lld.axis << " " << lld.delta;
}

void TraceWriter::LMove(const LinearDelta& sld1, const LinearDelta& sld2) {
  CHECK(-5 <= sld1.delta && sld1.delta <= 5);
  CHECK(-5 <= sld2.delta && sld2.delta <= 5);
  WriteByte(0b00001100 | ((static_cast<int>(sld1.axis) + 1) << 4) | ((static_cast<int>(sld2.axis) + 1) << 6));
  WriteByte((sld1.delta + 5) | ((sld2.delta + 5) << 4));
  //LOG(ERROR) << "LMove " << (int)sld1.axis << " " << sld1.delta << " " << (int)sld2.axis << " " << sld2.delta;
}

void TraceWriter::Fill(const Delta& nd) {
  WriteByte(0b00000011 | ((((nd.dx + 1) * 9 + (nd.dy + 1) * 3 + (nd.dz + 1))) << 3));
  //LOG(ERROR) << "Fill";
}

void TraceWriter::WriteByte(uint8_t b) {
  os_.write(reinterpret_cast<char*>(&b), 1);
}
