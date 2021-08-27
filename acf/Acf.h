// Solargene (c) Alexander Semenov 2020-2021
#pragma once
#include <CoreMinimal.h>
#include <Config.h>
#include <pos.h>
#include <fstream>

class AcfException {
};

//==============================================================================================
// формат данных ACF
//==============================================================================================
class Acf {
private:
  enum Type {
    NONE,
    BOOL,
    INT,
    FLOAT,
    STRING,
    FVECTOR,
    FROTATOR,
    CAGE_POS,
    GEO_POS,
    ARRAY,
    EMPTY_MAP,
    STR_MAP,
    INT_MAP,
    POS2_MAP,
    POS3_MAP,
    EMPTY_SET,
    STR_SET,
    BYTES,
  };

public:
  //====================================================================================================
  Acf(Type _type=NONE) 
    :type(_type)
  {
  }

  //====================================================================================================
  Acf(std::istream &file, int version, TMap<int, FString> &keys) {
    empty();
    load(file, version, keys);
  }

  //====================================================================================================
  virtual ~Acf() {
    empty();
  }

  //====================================================================================================
  bool load(const FString &filename);

  //====================================================================================================
  void load(std::istream &file, int version, TMap<int, FString> &keys);

  //====================================================================================================
  bool save(const FString &filename);

  //====================================================================================================
  void save(std::ostream &file, TMap<FString, int> &keys);

  //====================================================================================================
  inline bool isNone() const {
    return type == NONE;
  }

  //====================================================================================================
  inline bool isBool() const {
    return type == BOOL;
  }

  //====================================================================================================
  inline bool isInt() const {
    return type == INT;
  }

  //====================================================================================================
  inline bool isFloat() const {
    return type == FLOAT;
  }

  //====================================================================================================
  inline bool isString() const {
    return type == STRING;
  }

  //====================================================================================================
  inline bool isFVector() const {
    return type == FVECTOR;
  }

  //====================================================================================================
  inline bool isFRotator() const {
    return type == FROTATOR;
  }

  //====================================================================================================
  inline bool isCagePos() const {
    return type == CAGE_POS;
  }

  //====================================================================================================
  inline bool isGeoPos() const {
    return type == GEO_POS;
  }

  //====================================================================================================
  inline bool isArray() const {
    return type == ARRAY;
  }

  //====================================================================================================
  inline bool isMap() const {
    return type == EMPTY_MAP;
  }

  //====================================================================================================
  inline bool isIntMap() const {
    return type == EMPTY_MAP || type==INT_MAP;
  }

  //====================================================================================================
  inline bool isStrMap() const {
    return type == EMPTY_MAP || type==STR_MAP;
  }

  //====================================================================================================
  inline bool isPos2Map() const {
    return type == EMPTY_MAP || type==POS2_MAP;
  }

  //====================================================================================================
  inline bool isPos3Map() const {
    return type == EMPTY_MAP || type==POS3_MAP;
  }

  //====================================================================================================
  inline bool isStrSet() const {
    return type == EMPTY_SET || type==STR_SET;
  }

  //====================================================================================================
  inline bool isBytes() const {
    return type == BYTES;
  }

  //====================================================================================================
  operator bool() const;

  //====================================================================================================
  inline operator int() const {
    check(type == INT);
    return ac_int;
  }

  //====================================================================================================
  inline operator long long() const {
    check(type == INT);
    return ac_int;
  }

  //====================================================================================================
  inline operator Uid() const {
    check(type == INT);
    return ac_int;
  }

  //====================================================================================================
  inline operator float() const {
    check(type == FLOAT);
    return ac_float;
  }

  //====================================================================================================
  inline operator double() const {
    check(type == FLOAT);
    return ac_float;
  }

  //====================================================================================================
  inline operator FString &() {
    check(type == STRING);
    return ac_str;
  }

  //====================================================================================================
  inline operator FVector &() {
    check(type == FVECTOR);
    return ac_fvector;
  }

  //====================================================================================================
  inline operator FRotator &() {
    check(type == FROTATOR);
    return ac_frotator;
  }

  //====================================================================================================
  inline operator CagePos &() {
    check(type == CAGE_POS);
    return ac_cage_pos;
  }

  //====================================================================================================
  inline operator GeoPos &() {
    check(type == GEO_POS);
    return ac_geo_pos;
  }

  //====================================================================================================
  inline bool get(const FString &name, bool def) const {
    check(type == STR_MAP);
    auto value=str_map.Find(name);
    if (value && (*value)->type==BOOL)
      return (*value)->ac_bool;
    return def;
  }

  //====================================================================================================
  inline TArray<Acf*> &asArray(){
    if (type == NONE)
      type = ARRAY;
    check(type == ARRAY);
    return array;
  }

  //====================================================================================================
  template<class T>
  inline TArray<T> asArrayOf(){
    if (type == NONE)
      type = ARRAY;
    check(type == ARRAY);
    TArray<T> result;
    result.Reserve(array.Num());
    for (auto i : array)
      result.Add(*i);
    return result;
  }

  //====================================================================================================
  template<class T>
  inline TMap<FString, T> asStrMapOf(){
    if (type == NONE)
      type = STR_MAP;
    check(type == STR_MAP);
    TMap<FString, T> result;
    for (auto i : str_map)
      result.Add(i.Key, (T)*i.Value);
    return result;
  }

  //====================================================================================================
  inline TMap<FString, Acf*> &asStrMap(){
    if (type == NONE)
      type = STR_MAP;
    check(type == STR_MAP);
    return str_map;
  }

  //====================================================================================================
  inline TMap<int64, Acf*> &asIntMap(){
    if (type == NONE)
      type = INT_MAP;
    check(type == INT_MAP);
    return int_map;
  }

  //====================================================================================================
  inline TMap<Pos2D, Acf*> &asPos2Map(){
    if (type == NONE)
      type = POS2_MAP;
    check(type == POS2_MAP);
    return pos2_map;
  }

  //====================================================================================================
  inline TMap<Pos3D, Acf*> &asPos3Map(){
    if (type == NONE)
      type = POS3_MAP;
    check(type == POS3_MAP);
    return pos3_map;
  }

  //====================================================================================================
  inline TSet<FString> &asStrSet(){
    if (type == NONE)
      type = STR_SET;
    check(type == STR_SET);
    return str_set;
  }

  //====================================================================================================
  inline TArray<BYTE> &asBytes() {
    if (type == NONE)
      type = BYTES;
    check(type == BYTES);
    return bytes;
  }

  //====================================================================================================
  Acf &operator[](int index);

  //====================================================================================================
  Acf &operator[](const char *name) {
    return (*this)[FString(name)];
  }

  //====================================================================================================
  Acf &operator[](const FString &name);

  //====================================================================================================
  Acf &operator[](const Pos2D &pos);

  //====================================================================================================
  Acf &operator[](const Pos3D &pos);

  //====================================================================================================
  void empty();

  //====================================================================================================
  inline Acf &operator=(bool value) {
    check(type == NONE || type == BOOL);
    type = BOOL;
    ac_bool = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(int value) {
    check(type == NONE || type == INT);
    type = INT;
    ac_int = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(int64 value) {
    check(type == NONE || type == INT);
    type = INT;
    ac_int = value;
    return *this;
  }
  /*
  //====================================================================================================
  inline Acf &operator=(Uid value) {
    check(type == NONE || type == INT);
    type = INT;
    ac_int = value;
    return *this;
  }
  */

  //====================================================================================================
  inline Acf &operator=(float value) {
    check(type == NONE || type == FLOAT);
    type = FLOAT;
    ac_float = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(double value) {
    check(type == NONE || type == FLOAT);
    type = FLOAT;
    ac_float = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const FString &value) {
    check(type == NONE || type == STRING);
    type = STRING;
    ac_str = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const char *value) {
    check(type == NONE || type == STRING);
    type = STRING;
    ac_str = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const FVector &value) {
    check(type == NONE || type == FVECTOR);
    type = FVECTOR;
    ac_fvector = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const FRotator &value) {
    check(type == NONE || type == FROTATOR);
    type = FROTATOR;
    ac_frotator = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const CagePos &value) {
    check(type == NONE || type == CAGE_POS);
    type = CAGE_POS;
    ac_cage_pos = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const GeoPos &value) {
    check(type == NONE || type == GEO_POS);
    type = GEO_POS;
    ac_geo_pos = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const TSet<FString> &value) {
    empty();
    type = STR_SET;
    str_set = value;
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(const TArray<BYTE> &value) {
    empty();
    type = BYTES;
    bytes = value;
    return *this;
  }

  //====================================================================================================
  template<class T>
  inline Acf &operator=(const TArray<T> &value) {
    empty();
    type = ARRAY;
    array.Reserve(value.Num());
    for (auto i : value) {
      auto item = new Acf();
      array.Add(item);
      *item = i;
    }
    return *this;
  }

  //====================================================================================================
  template<class T>
  inline Acf &operator=(const TMap<FString, T> &value) {
    empty();
    type = STR_MAP;
    for (auto i : value) {
      auto item = new Acf();
      str_map.Add(i.Key, item);
      *item = i.Value;
    }
    return *this;
  }

  //====================================================================================================
  inline Acf &operator=(nullptr_t value) {
    empty();
    return *this;
  }

  //====================================================================================================
  inline bool operator==(nullptr_t value) {
    return type == NONE;
  }

  //====================================================================================================
  inline Acf &addToArray() {    
    asArray();
    Acf *item = new Acf;
    array.Add(item);
    return *item;
  }

private:
  void writeChar(std::ostream &file, char value);
  void writeInt(std::ostream &file, long long value);
  void writeString(std::ostream &file, const FString &value);
  void writeKey(std::ostream &file, const FString &key, TMap<FString, int> &keys);
  void writeFloat(std::ostream &file, float value);
  void writeDouble(std::ostream &file, double value);
  void writeBytes(std::ostream &file, const TArray<BYTE> &value);

  char readChar(std::istream &file);
  long long readInt(std::istream &file);
  float readFloat(std::istream &file);
  double readDouble(std::istream &file);
  FString readString(std::istream &file);
  FString readKey(std::istream &file, TMap<int, FString> &keys);
  TArray<BYTE> readBytes(std::istream &file);
  void readSkip(std::istream &file, long long count);

  Type                type=NONE;
  bool                ac_bool=false;
  long long           ac_int=0;
  double              ac_float=0;
  FString             ac_str;
  FVector             ac_fvector;
  FRotator            ac_frotator;
  CagePos             ac_cage_pos;
  GeoPos              ac_geo_pos;
  TArray<Acf*>        array;
  TMap<FString, Acf*> str_map;
  TMap<int64, Acf*>   int_map;
  TMap<Pos2D, Acf*>   pos2_map;
  TMap<Pos3D, Acf*>   pos3_map;
  TSet<FString>       str_set;
  TArray<BYTE>        bytes;
};

