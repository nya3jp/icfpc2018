#include "solver/io/model_reader.h"

#include "glog/logging.h"

Model ReadModel(std::istream& is) {
  int resolution;
  {
    uint8_t b;
    is.read(reinterpret_cast<char*>(&b), sizeof(b));
    resolution = b;
  }
  CHECK(is);

  std::vector<uint8_t> buf((resolution * resolution * resolution + 7) / 8);
  is.read(reinterpret_cast<char*>(buf.data()), buf.size());
  CHECK(is);

  std::vector<bool> data(resolution * resolution * resolution);
  for (uint32_t x = 0; x < resolution; ++x) {
    for (uint32_t y = 0; y < resolution; ++y) {
      for (uint32_t z = 0; z < resolution; ++z) {
        uint32_t i = x * resolution * resolution + y * resolution + z;
        data[i] = (buf[i / 8] & (1u << (i % 8))) != 0;
      }
    }
  }

  return Model(resolution, std::move(data));
}
