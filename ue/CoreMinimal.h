// AC4FAR (c) Alexander Semenov 2020-2021
#pragma once
#include "../ac4far/framework.h"
#include <map>
#include <string>
#include <cassert>
#include <vector>
#include <set>

class FString: public std::wstring {
public:
  FString() {}

  FString(const std::wstring &str) 
    :std::wstring(str)
  {}

  FString(const wchar_t *str) 
    :std::wstring(str)
  {}

  FString(const char *str) 
    :std::wstring(utf8ToWchar(std::string(str)))
  {}

  FString &operator=(const char *str) {
    (*this) = utf8ToWchar(std::string(str));
    return *this;
  }

  const wchar_t *operator*() const{
    return c_str();
  }

  //---------------------------------------------------------------------------------------------------------
  static inline std::string wcharToUtf8(const wchar_t * src) {
    int src_length = (int)wcslen(src);
    int length = WideCharToMultiByte(CP_UTF8, 0, src, src_length, 0, 0, 0, 0);
    std::string output_buffer;
    output_buffer.resize(length);
    WideCharToMultiByte(CP_UTF8, 0, src, src_length, (char*)output_buffer.data(), length, 0, 0);
    return output_buffer;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  static inline std::wstring utf8ToWchar(const std::string &src) {
    int src_length = (int)src.length();
    int length = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src_length, 0, 0);
    std::wstring out;
    out.resize(length);
    MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src_length, (wchar_t*)out.data(), length);
    return out;
  }
};

inline bool operator!=(const FString &str1, const char *str2) {
  return str1 != FString::utf8ToWchar(std::string(str2));
}



class FTCHARToUTF8 {
public:
  FTCHARToUTF8(const wchar_t *_str) {
    str = FString::wcharToUtf8(_str);
  }

  size_t Length() {
    return str.size();
  }

  const char *Get() {
    return str.c_str();
  }

  std::string str;
};


inline FString UTF8_TO_TCHAR(const char *utf8) {
  return FString::utf8ToWchar(std::string(utf8));
}

template<class Key, class Value>
class TMap: public std::map<Key, Value> {
public:
  Value *Find(const Key &key) const {
    auto i = this->find(key);
    if (i == this->end())
      return nullptr;
    return (Value*)&((*i).second);
  }

  size_t Num() const {
    return this->size();
  }

  Value &Add(const Key &key, Value value) {
    return (*this)[key] = value;
  }

  void Empty() {
    this->clear();
  }
};

#define Key first
#define Value second


template<class Value>
class TArray: public std::vector<Value> {
public:
  void Add(const Value &value) {
    this->push_back(value);
  }

  size_t Num() const {
    return this->size();
  }

  void Empty() {
    this->clear();
  }

  const Value *GetData() const {
    return this->data();
  }

  void Reserve(int count) {
    this->reserve(count);
  }

  void AddZeroed(int count) {
    while (count-- > 0)
      this->push_back(0);
  }
};

template<class Value>
class TSet: public std::set<Value> {
public:
  size_t Num() const {
    return this->size();
  }

  void Empty() {
    this->clear();
  }

  void Add(const Value &value) {
    this->insert(value);
  }
};



inline void check(bool value) {
  assert(value);
}



typedef unsigned long long Uid;
typedef uint64_t int64;
typedef uint8_t BYTE;



struct FVector {
  FVector(float _X=0, float _Y=0, float _Z=0)
    :X(_X)
    ,Y(_Y)
    ,Z(_Z) 
  {}

  float X, Y, Z;
};



struct FRotator {
  FRotator(float _Roll=0, float _Pitch=0, float _Yaw=0)
    :Roll(_Roll)
    ,Pitch(_Pitch)
    ,Yaw(_Yaw) 
  {}

  float Roll, Pitch, Yaw;
};
