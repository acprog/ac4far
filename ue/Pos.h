// AC4FAR (c) Alexander Semenov 2020-2021
#pragma once

struct Pos2D {
  int x, y;

  Pos2D(int _x = 0, int _y = 0)
    :x(_x)
    ,y(_y)
  {}
};

inline bool operator<(const Pos2D &a, const Pos2D &b) {
  return a.x * a.y < b.x *b.y;
}


struct Pos3D : Pos2D {
  int z;

  Pos3D(int _x = 0, int _y = 0, int _z = 0)
    :Pos2D(_x, _y)
    ,z(_z) 
  {}
};

struct CagePos : Pos3D {
};

inline bool operator<(const Pos3D &a, const Pos3D &b) {
  return a.x * a.y * a.z < b.x *b.y *b.z;
}

struct GeoPos {
  double latitude, longitude, elevation;
};

