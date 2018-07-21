#include "solver/model.h"

#include <stdint.h>

#include "glog/logging.h"

// static
std::unique_ptr<Model> Model::FromStream(std::istream& is) {
  int n;
  {
    uint8_t b;
    is.read(reinterpret_cast<char*>(&b), sizeof(b));
    n = b;
  }
  CHECK(is);

  std::vector<uint8_t> buf((n * n * n + 7) / 8);
  is.read(reinterpret_cast<char*>(buf.data()), buf.size());
  CHECK(is);

  std::vector<bool> data(n * n * n);
  for (uint32_t x = 0; x < n; ++x) {
    for (uint32_t y = 0; y < n; ++y) {
      for (uint32_t z = 0; z < n; ++z) {
        uint32_t i = x * n * n + y * n + z;
        // ret[x, y, z] = bool(data[i // 8] & (1 << (i % 8)))
        data[i] = (buf[i / 8] & (1u << (i % 8))) != 0;
      }
    }
  }

  return std::unique_ptr<Model>(new Model(n, std::move(data)));
}

bool Model::Get(int x, int y, int z) const {
  //LOG(ERROR) << x << " " << y << " " << z;
  return data_[x * n_ * n_ + y * n_ + z];
}
