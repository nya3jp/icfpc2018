#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <utility>

#include "solver/data/geometry.h"

enum CmdType{
  CMD_HALT,
  CMD_WAIT,
  CMD_FLIP,
  CMD_LMOVE,
  CMD_SMOVE,
  CMD_FILL,
  CMD_VOID,
  CMD_GFILL,
  CMD_GVOID,
  CMD_FISSION,
  CMD_FUSION_MASTER,
  CMD_FUSION_SLAVE,
};

struct Command{
  CmdType type;
  LinearDelta ld1;
  LinearDelta ld2;
  Delta nd;
  Delta fd;
  int arg;
};

struct VCE{
  // inclusive
  int minx, miny, minz;
  int maxx, maxy, maxz;
  explicit VCE(Point& c){
    minx = maxx = c.x;
    miny = maxy = c.y;
    minz = maxz = c.z;
  }
  explicit VCE(Point& c, LinearDelta& o){
    if(o.delta < 0){
      int neighbor = -1;
      minx = c.x + (o.axis == Axis::X ? o.delta : 0);
      maxx = c.x + (o.axis == Axis::X ? neighbor : 0);
      miny = c.y + (o.axis == Axis::Y ? o.delta : 0);
      maxy = c.y + (o.axis == Axis::Y ? neighbor : 0);
      minz = c.z + (o.axis == Axis::Z ? o.delta : 0);
      maxz = c.z + (o.axis == Axis::Z ? neighbor : 0);
    }else{
      int neighbor = 1;
      maxx = c.x + (o.axis == Axis::X ? o.delta : 0);
      minx = c.x + (o.axis == Axis::X ? neighbor : 0);
      maxy = c.y + (o.axis == Axis::Y ? o.delta : 0);
      miny = c.y + (o.axis == Axis::Y ? neighbor : 0);
      maxz = c.z + (o.axis == Axis::Z ? o.delta : 0);
      minz = c.z + (o.axis == Axis::Z ? neighbor : 0);
    }
  }
  explicit VCE(Region& r){
    minx = r.mini.x, miny = r.mini.y, minz = r.mini.z;
    maxx = r.maxi.x, maxy = r.maxi.y, maxz = r.maxi.z;
  }
  bool overlap(VCE& o){
    return (minx <= o.maxx && o.minx <= maxx) &&
           (miny <= o.maxy && o.miny <= maxy) &&
           (minz <= o.maxz && o.minz <= maxz);
  }
};

typedef std::priority_queue<int, std::vector<int>, std::greater<int> > Seeds;

struct Bot{
  int _bid;
  Point _pos;
  Seeds _seeds;
  explicit Bot(int bid, Point pos, Seeds seeds):
    _bid(bid), _pos(pos), _seeds(seeds){}
  VCE move(LinearDelta& delta){
    Point newpos = _pos + delta.ToDelta();
    _pos = std::move(newpos);
    return VCE(_pos, delta);
  }
  std::pair<VCE, Point> modify(Delta& delta){
    Point newpos = _pos + delta;
    return std::make_pair(VCE(newpos), newpos);
  }
  std::pair<Region, Point> gmodify(Delta& nd, Delta& fd){
    Point corner = _pos + nd;
    return std::make_pair(Region::FromPointDelta(corner, fd), corner);
  }
  std::pair<VCE, Bot> fission(Delta& delta, int m){
    if(_seeds.size() < m + 1){
      throw std::runtime_error("fission: child #seeds too large");
    }
    int newbid = _seeds.top();
    _seeds.pop();
    Point newpos = _pos + delta;
    Seeds newseeds;
    for(int i = 0; i < m; ++i){
      newseeds.push(_seeds.top());
      _seeds.pop();
    }
    Bot newbot(newbid, newpos, newseeds);
    return std::make_pair(VCE(newpos), newbot);
  }
  Point fusion_pre(Delta& delta){
    return _pos + delta;
  }
  void fusion_post(Bot& bot){
    _seeds.push(bot._bid);
    while(!bot._seeds.empty()){
      _seeds.push(bot._seeds.top());
      bot._seeds.pop();
    }
  }
  bool operator<(const Bot& o) const{
    return _bid > o._bid;
  }
};

struct Matrix{
  int N;
  int nfloat = 0;
  std::vector<std::vector<std::vector<int8_t> > > field;
  Matrix() = default;
  explicit Matrix(int resolution): N(resolution){
    resize();
  }
  void resize(){
    field.resize(N);
    for(auto& x : field){
      x.resize(N);
      for(auto& y : x){
        y.resize(N);
      }
    }
  }
  bool movable(const Point& c) const{
    return 0 <= c.x && c.x < N && 0 <= c.y && c.y < N && 0 <= c.z && c.z < N &&
           field[c.x][c.y][c.z] == 0;
  }
  bool placeable(const Point& mini, const Point& maxi) const{
    return 1 <= mini.x && maxi.x < N - 1 && 0 <= mini.y && maxi.y < N - 1 && 1 <= mini.z && maxi.z < N - 1;
  }
  bool placeable(const Point& c) const{
    return placeable(c, c);
  }
  bool placeable(const Region& r) const{
    return placeable(r.mini, r.maxi);
  }
  bool fill(const Point& c){
    if(!placeable(c)){
      throw std::runtime_error("fill out of field");
    }
    if(field[c.x][c.y][c.z] != 0){
      std::cout << "(" << c.x << ", " << c.y << ", " << c.z << ") already filled" << std::endl;
      return false;
    }
    if(c.y == 0 ||
       field[c.x-1][c.y][c.z] == 1 || field[c.x+1][c.y][c.z] == 1 ||
       field[c.x][c.y-1][c.z] == 1 || field[c.x][c.y+1][c.z] == 1 ||
       field[c.x][c.y][c.z-1] == 1 || field[c.x][c.y][c.z+1] == 1){
      field[c.x][c.y][c.z] = 1;
      bfs(c);
    }else{
      field[c.x][c.y][c.z] = 1;  // should be -1 after implementing union find
      // ++nfloat;  // TODO: implement bfs
    }
    return true;
  }
  bool void_(const Point& c){
    if(!placeable(c)){
      throw std::runtime_error("void out of field");
    }
    if(field[c.x][c.y][c.z] == 0){
      std::cout << "(" << c.x << ", " << c.y << ", " << c.z << ") already voided" << std::endl;
      return false;
    }
    field[c.x][c.y][c.z] = 0;
    // TODO: bridge detection???
    return true;
  }
  std::pair<int, int> gfill(const Region& r){
    if(!placeable(r)){
      throw std::runtime_error("gfill out of field");
    }
    int changed = 0;
    int unchanged = 0;
    // TODO: union-find here
    for(int x = r.mini.x; x <= r.maxi.x; ++x)
    for(int y = r.mini.y; y <= r.maxi.y; ++y)
    for(int z = r.mini.z; z <= r.maxi.z; ++z){
      if(field[x][y][z] == 0){
        field[x][y][z] = 1;  // TODO: value
        ++changed;
      }else{
        ++unchanged;
      }
    }
    return std::make_pair(changed, unchanged);
  }
  std::pair<int, int> gvoid(const Region& r){
    if(!placeable(r)){
      throw std::runtime_error("gvoid out of field");
    }
    int changed = 0;
    int unchanged = 0;
    // TODO: some algorithm
    for(int x = r.mini.x; x <= r.maxi.x; ++x)
    for(int y = r.mini.y; y <= r.maxi.y; ++y)
    for(int z = r.mini.z; z <= r.maxi.z; ++z){
      if(field[x][y][z] != 0){
        field[x][y][z] = 0;
        ++changed;
      }else{
        ++unchanged;
      }
    }
    return std::make_pair(changed, unchanged);
  }
  inline bool floating(){
    return nfloat > 0;
  }
  void bfs(const Point& c){
    // TODO
  }
  bool load(const char *filename){
    std::ifstream mdl;
    mdl.open(filename, std::ios::in | std::ios::binary);
    if(!mdl){
      std::cerr << filename << " cannot be opened" << std::endl;
      return false;
    }
    uint8_t resolution;
    mdl.read((char *)&resolution, 1);
    N = resolution;
    resize();
    uint8_t data;
    uint8_t pos = 0;
    for(auto& yz : field){
      for(auto& z : yz){
        for(auto&& pixel : z){
          if(pos == 0){
            mdl.read((char *)&data, 1);
            if(mdl.eof()){
              std::cerr << "not enough data in model" << std::endl;
              return false;
            }
          }
          pixel = (data >> pos) & 1;
          pos = (pos + 1) % 8;
        }
      }
    }
    mdl.read((char *)&data, 1);
    if(!mdl.eof()){
      std::cerr << "data remains in model" << std::endl;
    }
    mdl.close();
    return true;
  }
  // for debug
  void dump(const char *filename){
    std::ofstream mdl;
    mdl.open(filename, std::ios::out | std::ios::binary);
    if(!mdl){
      std::cerr << filename << " cannot be opened" << std::endl;
      return;
    }
    uint8_t resolution = N;
    mdl.write((char *)&resolution, 1);
    uint8_t data = 0;
    int pos = 0;
    for(auto& yz : field){
      for(auto& z : yz){
        for(auto& pixel : z){
          if(pixel != 0){
            data |= 1 << pos;
          }
          if(++pos == 8){
            mdl.write((char *)&data, 1);
            data = 0;
            pos = 0;
          }
        }
      }
    }
    if(pos > 0){
      mdl.write((char *)&data, 1);
    }
    mdl.close();
  }
};

struct State{
  uint64_t _energy;
  bool _harmonics;
  Matrix _m;
  std::priority_queue<Bot> _bots;
  std::queue<Command> _trace;
  explicit State(uint64_t energy,
                 bool harmonics,
                 Matrix& m,
                 std::priority_queue<Bot>& bots,
                 std::queue<Command>& trace):
    _energy(energy), _harmonics(harmonics), _m(m), _bots(bots), _trace(trace){}
  void run(){
    int nsteps = 0;
    while(!_bots.empty()){
      _energy += (_harmonics ? 30 : 3) * _m.N * _m.N * _m.N;
      _energy += 20 * _bots.size();
      step();
      ++nsteps;
    }
    std::cout << nsteps << " " << _energy << std::endl;
  }
  void step(){
    int nbots = _bots.size();
    std::priority_queue<Bot> newbots;
    std::map<std::pair<Point, Point>, Bot> masters, slaves;
    std::map<Region, std::set<Point> > gfills, gvoids;
    std::vector<VCE> vcs;
    while(!_bots.empty()){
      Bot bot = _bots.top();
      _bots.pop();
      Command cmd = _trace.front();
      _trace.pop();
      vcs.emplace_back(VCE(bot._pos));
      switch(cmd.type){
      case CMD_HALT:
        if(nbots != 1 || !bot._pos.IsOrigin() || _harmonics || _trace.size() > 0){
          std::cout << nbots << " " << bot._pos.x << " " << bot._pos.y << " " << bot._pos.z << " " << _harmonics << " " << _trace.size() << std::endl;
          throw std::runtime_error("halt");
        }
        return;

      case CMD_FLIP:
        _harmonics = !_harmonics;
        if(!_harmonics && _m.floating()){
          throw std::runtime_error("cannot change to low harmonics due to floating objects");
        }
        break;

      case CMD_SMOVE:
        {
          VCE vce = bot.move(cmd.ld1);
          if(!_m.movable(bot._pos)){
            throw std::runtime_error("move out of field");
          }
          vcs.emplace_back(vce);
          _energy += 2 * std::abs(cmd.ld1.delta);
          break;
        }

      case CMD_LMOVE:
        {
          VCE vce = bot.move(cmd.ld1);
          if(!_m.movable(bot._pos)){
            throw std::runtime_error("move out of field");
          }
          vcs.emplace_back(vce);
          vce = bot.move(cmd.ld2);
          if(!_m.movable(bot._pos)){
            throw std::runtime_error("move out of field");
          }
          vcs.emplace_back(vce);
          _energy += 2 * (std::abs(cmd.ld1.delta) + 2 + std::abs(cmd.ld2.delta));
          break;
        }

      case CMD_FILL:
        {
          std::pair<VCE, Point> res = bot.modify(cmd.nd);
          vcs.emplace_back(res.first);
          bool filled = _m.fill(res.second);
          if(!_harmonics && _m.floating()){
            throw std::runtime_error("cannot place floating objects");
          }
          _energy += filled ? 12 : 6;
          break;
        }

      case CMD_VOID:
        {
          std::pair<VCE, Point> res = bot.modify(cmd.nd);
          vcs.emplace_back(res.first);
          bool voided = _m.void_(res.second);
          if(!_harmonics && _m.floating()){
            throw std::runtime_error("float after void");
          }
          _energy += voided ? -12 : 3;
          break;
        }

      case CMD_GFILL:
        {
          std::pair<Region, Point> res = bot.gmodify(cmd.nd, cmd.fd);
          auto it = gfills.find(res.first);
          if(it == gfills.end()){
            vcs.emplace_back(VCE(res.first));
            gfills.insert(std::make_pair(res.first, std::set<Point>({res.second})));
          }else{
            gfills[res.first].insert(res.second);
          }
          break;
        }

      case CMD_GVOID:
        {
          std::pair<Region, Point> res = bot.gmodify(cmd.nd, cmd.fd);
          auto it = gvoids.find(res.first);
          if(it == gvoids.end()){
            vcs.emplace_back(VCE(res.first));
            gvoids.insert(std::make_pair(res.first, std::set<Point>({res.second})));
          }else{
            gvoids[res.first].insert(res.second);
          }
          break;
        }

      case CMD_FISSION:
        {
          std::pair<VCE, Bot> res = bot.fission(cmd.nd, cmd.arg);
          if(!_m.movable(res.second._pos)){
            throw std::runtime_error("fission out of field");
          }
          vcs.emplace_back(res.first);
          newbots.push(res.second);
          _energy += 24;
          break;
        }

      case CMD_FUSION_SLAVE:
        {
          Point mpos = bot.fusion_pre(cmd.nd);
          slaves.insert(std::make_pair(std::make_pair(mpos, bot._pos), bot));
          continue;
        }

      case CMD_FUSION_MASTER:
        {
          Point spos = bot.fusion_pre(cmd.nd);
          masters.insert(std::make_pair(std::make_pair(bot._pos, spos), bot));
          _energy -= 24;
          break;
        }

      case CMD_WAIT:
        break;
      }
      newbots.push(bot);
    }
    for(int i = 0; i < (int)vcs.size(); ++i){
      for(int j = i + 1; j < (int)vcs.size(); ++j){
        if(vcs[i].overlap(vcs[j])){
          throw std::runtime_error("volatile coordinates overlap");
        }
      }
    }
    for(auto& kv : masters){
      auto& key = kv.first;
      auto& bot = kv.second;
      auto it = slaves.find(key);
      if(it == slaves.end()){
        throw std::runtime_error("fusion master/slave unmatch");
      }
      bot.fusion_post(it->second);
    }
    for(auto& kv : gfills){
      auto& region = kv.first;
      auto& points = kv.second;
      if(points.size() != (1 <<region.Dimension())){
        throw std::runtime_error("gfill #points unmatch");
      }
      std::pair<int, int> res = _m.gfill(region);
      _energy += res.first * 12 + res.second * 6;
    }
    for(auto& kv : gvoids){
      auto& region = kv.first;
      auto& points = kv.second;
      if(points.size() != (1 <<region.Dimension())){
        throw std::runtime_error("gvoid #points unmatch");
      }
      std::pair<int, int> res = _m.gvoid(region);
      _energy += res.first * -12 + res.second * 3;
    }
    _bots = std::move(newbots);
  }
  void mdump(const char *filename){
    _m.dump(filename);
  }
};

enum class Problem{
  Assembly,
  Disassembly,
  Reassembly,
};

int main(int argc, char *argv[]){
  if(argc < 3){
    std::cerr << "Usage: " << argv[0] << " <mdl> <nbt> [<mdl>]" << std::endl;
    return 1;
  }

  // TODO: option
  Problem problem;
  switch(*(strrchr(argv[1], '/') ? strrchr(argv[1], '/') + 2 : argv[1] + 1)){
  case 'A':
    problem = Problem::Assembly;
    break;
  case 'D':
    problem = Problem::Disassembly;
    break;
  case 'R':
    problem = Problem::Reassembly;
    break;
  default:
    problem = Problem::Assembly;
  }

  Matrix m0;
  if(!m0.load(argv[1])){
    return 1;
  }
  Matrix m1(m0.N);
  if(problem == Problem::Reassembly){
    if(argc < 4 || !m1.load(argv[3])){
      return 1;
    }
  }
  if(problem == Problem::Assembly){
    std::swap(m0, m1);
  }

  std::ifstream nbt;
  nbt.open(argv[2], std::ios::in | std::ios::binary);
  if(!nbt){
    std::cerr << "cannot open files" << std::endl;
    return 1;
  }

  std::queue<Command> trace;
  while(true){
    Command cmd;
    uint8_t code0;
    uint8_t code1;
    nbt.read((char *)&code0, 1);
    if(nbt.eof()){
      break;
    }
    if(code0 == 0xff){
      cmd.type = CMD_HALT;
    }else if(code0 == 0xfe){
      cmd.type = CMD_WAIT;
    }else if(code0 == 0xfd){
      cmd.type = CMD_FLIP;
    }else{
      uint8_t subcode = code0 & 7;
      if(subcode == 4){
        LinearDelta ld1, ld2;
        nbt.read((char *)&code1, 1);
        int axis1 = (int)((code0 >> 4) & 3) - 1;
        int axis2 = (int)((code0 >> 6) & 3) - 1;
        if(code0 & 8){
          int delta1 = (int)(code1 & 0xf) - 5;
          int delta2 = (int)((code1 >> 4) & 0xf) - 5;
          if(delta1 == 0 || delta1 > 5){
            std::cerr << "invalid sld1 at pos " << nbt.tellg() << std::endl;
            return 1;
          }
          if(delta2 == 0 || delta2 > 5){
            std::cerr << "invalid sld2 at pos " << nbt.tellg() << std::endl;
            return 1;
          }
          cmd.type = CMD_LMOVE;
          cmd.ld1 = LinearDelta(static_cast<Axis>(axis1), delta1);
          cmd.ld2 = LinearDelta(static_cast<Axis>(axis2), delta2);
        }else{
          int delta1 = (int)code1 - 15;
          if(delta1 == 0 || delta1 > 15){
            std::cerr << "invalid lld at pos " << nbt.tellg() << std::endl;
            return 1;
          }
          cmd.type = CMD_SMOVE;
          cmd.ld1 = LinearDelta(static_cast<Axis>(axis1), delta1);
        }
      }else{
        int ndcode = code0 >> 3;
        int dx = ndcode / 9 - 1;
        int dy = ndcode % 9 / 3 - 1;
        int dz = ndcode % 3 - 1;
        if((dx == 0 && dy == 0 && dz == 0) ||
           (dx != 0 && dy != 0 && dz != 0)){
          std::cerr << "invalid nd at pos " << nbt.tellg() << std::endl;
          return 1;
        }
        cmd.nd = Delta(dx, dy, dz);
        if(subcode == 7){
          cmd.type = CMD_FUSION_MASTER;
        }else if(subcode == 6){
          cmd.type = CMD_FUSION_SLAVE;
        }else if(subcode == 5){
          nbt.read((char *)&code1, 1);
          cmd.type = CMD_FISSION;
          cmd.arg = code1;
        }else if(subcode == 3){
          cmd.type = CMD_FILL;
        }else if(subcode == 2){
          cmd.type = CMD_VOID;
        }else{
          nbt.read((char *)&code1, 1);
          int dx = (int)code1 - 30;
          nbt.read((char *)&code1, 1);
          int dy = (int)code1 - 30;
          nbt.read((char *)&code1, 1);
          int dz = (int)code1 - 30;
          if((dx == 0 && dy == 0 && dz == 0) ||
             dx > 30 || dy > 30 || dz > 30){
            std::cerr << "invalid fd at pos " << nbt.tellg() << std::endl;
            return 1;
          }
          cmd.fd = Delta(dx, dy, dz);
          if(subcode == 1){
            cmd.type = CMD_GFILL;
          }else{
            cmd.type = CMD_GVOID;
          }
        }
      }
    }
    trace.push(cmd);
  }
  nbt.close();

  uint64_t energy = 0;
  bool harmonics = false;
  std::priority_queue<Bot> bots;
  Seeds seeds;
  for(int i = 2; i < 41; ++i){
    seeds.push(i);
  }
  bots.push(Bot(1, Point(0, 0, 0), seeds));

  State s(energy, harmonics, m0, bots, trace);

  s.run();

  if(s._m.field != m1.field){
    std::cerr << "generated object differs from reference" << std::endl;
    return 1;
  }

  return 0;
}

