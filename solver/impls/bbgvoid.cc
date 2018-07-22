#include "solver/impls/bbgvoid.h"

#include <algorithm>
#include <utility>

#include "solver/data/geometry.h"

void BBGvoidSolver::Solve() {
  int n = model_->Resolution();

  int min_x = n, min_y = n, min_z = n, max_x = -1, max_y = -1, max_z = -1;
  for (int x = 0; x < n; ++x) {
    for (int y = 0; y < n; ++y) {
      for (int z = 0; z < n; ++z) {
        if (model_->Get(x, y, z)) {
          min_x = std::min(min_x, x);
          min_y = std::min(min_y, y);
          min_z = std::min(min_z, z);
          max_x = std::max(max_x, x);
          max_y = std::max(max_y, y);
          max_z = std::max(max_z, z);
        }
      }
    }
  }
  CHECK(max_x != min_x && max_y != min_y && max_z != min_z) << "too thin, cannot solve by BBGvoid";
  CHECK(max_x - min_x <= FAR_LEN && max_y - min_y <= FAR_LEN && max_z - min_z <= FAR_LEN) << "too large, cannot solve by BBGvoid";

  struct {
    Axis axis;
    int delta;
    bool fork_x;
    bool fork_z;
    bool fork_y;
    bool active;
    CarelessController* controller;
  } config[8] = {
    {Axis::X, -1, true, true, true, true, &controller_},
    {Axis::X, max_x + 1 - min_x, false, false, false, false},
    {Axis::Z, max_z + 1 - min_z, true, false, false, false},
    {Axis::X, max_x + 1 - min_x, false, false, false, false},
    {Axis::Y, max_y - min_y - 1, true, true, false, false},
    {Axis::X, max_x + 1 - min_x, false, false, false, false},
    {Axis::Z, max_z + 1 - min_z, true, false, false, false},
    {Axis::X, max_x + 1 - min_x, false, false, false, false},
  };

  if(min_x != 1 || min_z != 1){
    controller_.MoveTo(Point{min_x - 1, min_y, min_z - 1});
  }
  int nfinished = 0;
  while(nfinished < 8){
    std::vector<int> actives;
    for(int i = 0; i < 8; ++i){
      if(!config[i].active) continue;
      actives.push_back(i);
    }
    for(auto i : actives){
      if(config[i].delta > 0){
        int delta = std::min(config[i].delta, LONG_LEN);
        Delta d = LinearDelta{config[i].axis, delta}.ToDelta();
        config[i].controller->MoveDelta(d);
        config[i].delta -= delta;
        if(config[i].delta == 0 && !config[i].fork_x){
          ++nfinished;
        }
      }else if(config[i].fork_x){
        config[i + 1].controller = config[i].controller->Fission(Delta{1, 0, 0}, 0);
        config[i + 1].active = true;
        config[i].fork_x = false;
        if(!config[i].fork_z){
          ++nfinished;
        }
      }else if(config[i].fork_z){
        config[i + 2].controller = config[i].controller->Fission(Delta{0, 0, 1}, 1);
        config[i + 2].active = true;
        config[i].fork_z = false;
        if(!config[i].fork_y){
          ++nfinished;
        }
      }else if(config[i].fork_y){
        config[i + 4].controller = config[i].controller->Fission(Delta{0, 1, 0}, 3);
        config[i + 4].active = true;
        config[i].fork_y = false;
        ++nfinished;
      }else{
        config[i].controller->Wait();
      }
    }
  }
  int ix = 0;
  int fx = max_x - min_x;
  int fy = max_y - min_y;
  int fz = max_z - min_z;
  for(int dy = 1; dy > -2; dy -= 2){
    for(int dz = 1; dz > -2; dz -= 2){
      for(int dx = 1; dx > -2; dx -= 2){
        config[ix].controller->Gvoid(Delta{dx, 0, dz}, Delta{dx * fx, dy * fy, dz * fz});
        ++ix;
      }
    }
  }
  config[3].axis = Axis::Z;
  config[5].axis = config[6].axis = config[7].axis = Axis::Y;
  config[1].delta = max_x + 1 - min_x;
  config[2].delta = config[3].delta = max_z + 1 - min_z;
  config[4].delta = config[5].delta = config[6].delta = config[7].delta = max_y - min_y - 1;
  Axis axis = Axis::Y;
  while(true){
    std::vector<int> actives;
    for(int i = 0; i < 8; ++i){
      if(!config[i].active) continue;
      actives.push_back(i);
    }
    bool fusion = false;
    for(auto i : actives){
      if(config[i].axis == axis && config[i].delta >= 0){
        int delta = std::min(config[i].delta, LONG_LEN);
        Delta d = LinearDelta{config[i].axis, -delta}.ToDelta();
        config[i].controller->MoveDelta(d);
        config[i].delta -= delta;
        if(config[i].delta == 0){
          fusion = true;
        }
      }else{
        config[i].controller->Wait();
      }
    }
    if(!fusion) continue;
    for(auto i : actives){
      if(config[i].axis == axis && config[i].delta >= 0){
        config[i].controller->FusionS(LinearDelta(axis, -1).ToDelta());
        config[i].active = false;
      }else{
        config[i].controller->FusionP(LinearDelta{axis, 1}.ToDelta());
      }
    }
    if(axis == Axis::Y){
      axis = Axis::Z;
    }else if(axis == Axis::Z){
      axis = Axis::X;
    }else{
      break;
    }
  }
  controller_.MoveTo(Point{0, 0, 0});
  controller_.Halt();
}
