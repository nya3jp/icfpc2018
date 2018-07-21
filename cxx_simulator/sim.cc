#include <iostream>
#include <fstream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <map>
#include <queue>
#include <utility>

struct Delta{
  int axis;
  int delta;
};

struct NearDelta{
  int dx, dy, dz;
};

struct Coordinate{
  int x, y, z;
  Coordinate(int _x, int _y, int _z): x(_x), y(_y), z(_z){}
  const inline bool isorigin(){
    return x == 0 && y == 0 && z == 0;
  }
  inline Coordinate& operator+=(const Delta& o){
    x += o.axis == 0 ? o.delta : 0;
    y += o.axis == 1 ? o.delta : 0;
    z += o.axis == 2 ? o.delta : 0;
    return *this;
  }
  inline Coordinate& operator+=(const NearDelta& o){
    x += o.dx, y += o.dy, z += o.dz;
    return *this;
  }
  inline Coordinate operator+(const Delta& o) const{
    return Coordinate(*this) += o;
  }
  inline Coordinate operator+(const NearDelta& o) const{
    return Coordinate(*this) += o;
  }
  inline bool operator<(const Coordinate& o) const{
    return x < o.x || (x == o.x && (y < o.y || (y == o.y && z < o.z)));
  }
};

enum CmdType{
  CMD_HALT,
  CMD_WAIT,
  CMD_FLIP,
  CMD_LMOVE,
  CMD_SMOVE,
  CMD_FILL,
  CMD_FISSION,
  CMD_FUSION_MASTER,
  CMD_FUSION_SLAVE,
};

struct Command{
  CmdType type;
  Delta delta0;
  Delta delta1;
  NearDelta delta2;
  int arg;
};

struct VCE{
  // inclusive
  int minx, miny, minz;
  int maxx, maxy, maxz;
  explicit VCE(Coordinate& c){
    minx = maxx = c.x;
    miny = maxy = c.y;
    minz = maxz = c.z;
  }
  explicit VCE(Coordinate& c, Delta& o){
    if(o.delta < 0){
      int neighbor = -1;
      minx = c.x + (o.axis == 0 ? o.delta : 0);
      maxx = c.x + (o.axis == 0 ? neighbor : 0);
      miny = c.y + (o.axis == 1 ? o.delta : 0);
      maxy = c.y + (o.axis == 1 ? neighbor : 0);
      minz = c.z + (o.axis == 2 ? o.delta : 0);
      maxz = c.z + (o.axis == 2 ? neighbor : 0);
    }else{
      int neighbor = 1;
      maxx = c.x + (o.axis == 0 ? o.delta : 0);
      minx = c.x + (o.axis == 0 ? neighbor : 0);
      maxy = c.y + (o.axis == 1 ? o.delta : 0);
      miny = c.y + (o.axis == 1 ? neighbor : 0);
      maxz = c.z + (o.axis == 2 ? o.delta : 0);
      minz = c.z + (o.axis == 2 ? neighbor : 0);
    }
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
  Coordinate _pos;
  Seeds _seeds;
  explicit Bot(int bid, Coordinate pos, Seeds seeds):
    _bid(bid), _pos(pos), _seeds(seeds){}
  VCE move(Delta& delta){
    Coordinate newpos = _pos + delta;
    _pos = std::move(newpos);
    return VCE(_pos, delta);
  }
  std::pair<VCE, Coordinate> fill(NearDelta& delta){
    Coordinate newpos = _pos + delta;
    return std::make_pair(VCE(newpos), newpos);
  }
  std::pair<VCE, Bot> fission(NearDelta& delta, int m){
    if(_seeds.size() < m + 1){
      throw std::runtime_error("fission: child #seeds too large");
    }
    int newbid = _seeds.top();
    _seeds.pop();
    Coordinate newpos = _pos + delta;
    Seeds newseeds;
    for(int i = 0; i < m; ++i){
      newseeds.push(_seeds.top());
      _seeds.pop();
    }
    Bot newbot(newbid, newpos, newseeds);
    return std::make_pair(VCE(newpos), newbot);
  }
  Coordinate fusion_pre(NearDelta& delta){
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
  bool movable(Coordinate& c){
    return 0 <= c.x && c.x < N && 0 <= c.y && c.y < N && 0 <= c.z && c.z < N &&
           field[c.x][c.y][c.z] == 0;
  }
  bool fill(Coordinate& c){
    if(c.x < 1 || c.x > N - 2 || c.y < 0 || c.y > N - 2 || c.z < 0 || c.z > N - 2){
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
  inline bool floating(){
    return nfloat > 0;
  }
  void bfs(Coordinate& c){
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
    std::map<std::pair<Coordinate, Coordinate>, Bot> masters, slaves;
    std::vector<VCE> vcs;
    while(!_bots.empty()){
      Bot bot = _bots.top();
      _bots.pop();
      Command cmd = _trace.front();
      _trace.pop();
      vcs.emplace_back(VCE(bot._pos));
      switch(cmd.type){
      case CMD_HALT:
        if(nbots != 1 || !bot._pos.isorigin() || _harmonics || _trace.size() > 0){
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
          VCE vce = bot.move(cmd.delta0);
          if(!_m.movable(bot._pos)){
            throw std::runtime_error("move out of field");
          }
          vcs.emplace_back(vce);
          _energy += 2 * std::abs(cmd.delta0.delta);
          break;
        }

      case CMD_LMOVE:
        {
          VCE vce = bot.move(cmd.delta0);
          if(!_m.movable(bot._pos)){
            throw std::runtime_error("move out of field");
          }
          vcs.emplace_back(vce);
          vce = bot.move(cmd.delta1);
          if(!_m.movable(bot._pos)){
            throw std::runtime_error("move out of field");
          }
          vcs.emplace_back(vce);
          _energy += 2 * (std::abs(cmd.delta0.delta) + 2 + std::abs(cmd.delta1.delta));
          break;
        }

      case CMD_FILL:
        {
          std::pair<VCE, Coordinate> res = bot.fill(cmd.delta2);
          vcs.emplace_back(res.first);
          bool filled = _m.fill(res.second);
          if(!_harmonics && _m.floating()){
            throw std::runtime_error("cannot place floating objects");
          }
          _energy += filled ? 12 : 6;
          break;
        }

      case CMD_FISSION:
        {
          std::pair<VCE, Bot> res = bot.fission(cmd.delta2, cmd.arg);
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
          Coordinate mpos = bot.fusion_pre(cmd.delta2);
          slaves.insert(std::make_pair(std::make_pair(mpos, bot._pos), bot));
          continue;
        }

      case CMD_FUSION_MASTER:
        {
          Coordinate spos = bot.fusion_pre(cmd.delta2);
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
    _bots = std::move(newbots);
  }
  void mdump(const char *filename){
    _m.dump(filename);
  }
};

int main(int argc, char *argv[]){
  if(argc < 3){
    std::cerr << "Usage: " << argv[0] << " <mdl> <nbt>" << std::endl;
    return 1;
  }

  Matrix ref;
  if(!ref.load(argv[1])){
    return 1;
  }

  std::ifstream nbt;
  nbt.open(argv[2], std::ios::in | std::ios::binary);
  if(!nbt){
    std::cerr << "cannot open files" << std::endl;
    return 1;
  }

  std::queue<Command> trace;
  while(true){
    uint8_t code0;
    uint8_t code1;
    nbt.read((char *)&code0, 1);
    if(nbt.eof()){
      break;
    }
    if(code0 == 0xff){
      trace.push(Command{CMD_HALT});
    }else if(code0 == 0xfe){
      trace.push(Command{CMD_WAIT});
    }else if(code0 == 0xfd){
      trace.push(Command{CMD_FLIP});
    }else{
      uint8_t subcode = code0 & 7;
      if(subcode == 4){
        nbt.read((char *)&code1, 1);
        int axis1 = (int)((code0 >> 4) & 3) - 1;
        int axis2 = (int)((code0 >> 6) & 3) - 1;
        if(code0 & 8){
          int delta1 = (int)(code1 & 0xf) - 5;
          int delta2 = (int)((code1 >> 4) & 0xf) - 5;
          if(delta1 == 0 || delta1 > 5){
            std::cerr << "invalid sld1 at pos " << nbt.tellg() << std::endl;
          }
          if(delta2 == 0 || delta2 > 5){
            std::cerr << "invalid sld2 at pos " << nbt.tellg() << std::endl;
          }
          trace.push(Command{CMD_LMOVE, Delta{axis1, delta1}, Delta{axis2, delta2}});
        }else{
          int delta1 = (int)code1 - 15;
          if(delta1 == 0 || delta1 > 15){
            std::cerr << "invalid lld at pos " << nbt.tellg() << std::endl;
          }
          trace.push(Command{CMD_SMOVE, Delta{axis1, delta1}});
        }
      }else{
        int nd = code0 >> 3;
        int dx = nd / 9 - 1;
        int dy = nd % 9 / 3 - 1;
        int dz = nd % 3 - 1;
        if((dx == 0 && dy == 0 && dz == 0) ||
           (dx != 0 && dy != 0 && dz != 0)){
          std::cerr << "invalid nd at pos " << nbt.tellg() << std::endl;
          return 0;
        }
        NearDelta delta = {dx, dy, dz};
        Delta gomi = {0, 0};
        if(subcode == 7){
          trace.push(Command{CMD_FUSION_MASTER, gomi, gomi, delta});
        }else if(subcode == 6){
          trace.push(Command{CMD_FUSION_SLAVE, gomi, gomi, delta});
        }else if(subcode == 3){
          trace.push(Command{CMD_FILL, gomi, gomi, delta});
        }else if(subcode == 5){
          nbt.read((char *)&code1, 1);
          trace.push(Command{CMD_FISSION, gomi, gomi, delta, code1});
        }else{
          std::cerr << "invalid command at pos " << nbt.tellg() << std::endl;
          return 0;
        }
      }
    }
  }
  nbt.close();

  // lightning config
  uint64_t energy = 0;
  bool harmonics = false;
  Matrix m(ref.N);
  std::priority_queue<Bot> bots;
  Seeds seeds;
  for(int i = 2; i < 21; ++i){
    seeds.push(i);
  }
  bots.push(Bot(1, Coordinate(0, 0, 0), seeds));

  State s(energy, harmonics, m, bots, trace);

  s.run();

  if(s._m.field != ref.field){
    std::cerr << "generated object differs from reference" << std::endl;
    return 1;
  }

  return 0;
}

