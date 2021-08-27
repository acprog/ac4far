// Solargene (c) Alexander Semenov 2020-2021
#include "Acf.h"

static const int MAGIC='BCA\0';
static const int VERSION=1;

//==============================================================================================
// формат данных ACF
//==============================================================================================
Acf::operator bool() const {
  switch (type) {
    case NONE:      return false;
    case BOOL:      return ac_bool;
    case INT:       return ac_int != 0;
    case FLOAT:     return ac_float != 0;
    case STRING:    return ac_str != "";
    case ARRAY:     return array.Num() != 0;
    case EMPTY_MAP: return false;
    case STR_MAP:   return str_map.Num() != 0;
    case INT_MAP:   return int_map.Num() != 0;
    case POS2_MAP:  return pos2_map.Num() != 0;
    case POS3_MAP:  return pos3_map.Num() != 0;
    case EMPTY_SET: return false;
    case STR_SET:   return str_set.Num()!=0;
    case BYTES:     return bytes.Num() != 0;
  }
  return true;
}


//====================================================================================================
Acf &Acf::operator[](int index) {
  switch (type) {
    case ARRAY:   return *array[index];
    case NONE:
    case EMPTY_MAP:
      type = INT_MAP;
    case INT_MAP: {
      if (auto i = int_map.Find(index))
        return **i;
      return *int_map.Add(index, new Acf());
    }
  }
  check(false);
  return *(Acf*)nullptr;
}

//====================================================================================================
Acf &Acf::operator[](const FString &name) {
  if (type == EMPTY_MAP || type==NONE)
    type = STR_MAP;
  check(type == STR_MAP);
  if (auto i = str_map.Find(name))
    return **i;
  return *str_map.Add(name, new Acf());
}

//====================================================================================================
Acf &Acf::operator[](const Pos2D &pos) {
  if (type == EMPTY_MAP || type==NONE)
    type = POS2_MAP;
  check(type == POS2_MAP);
  if (auto i = pos2_map.Find(pos))
    return **i;
  return *pos2_map.Add(pos, new Acf());
}

//====================================================================================================
Acf &Acf::operator[](const Pos3D &pos) {
  if (type == EMPTY_MAP || type==NONE)
    type = POS3_MAP;
  check(type == POS3_MAP);
  if (auto i = pos3_map.Find(pos))
    return **i;
  return *pos3_map.Add(pos, new Acf());
}

//====================================================================================================
void Acf::empty() {
  type=NONE;
  ac_bool=false;
  ac_int=0;
  ac_float=0;
  ac_str="";
  ac_fvector=FVector(0, 0, 0);
  ac_frotator=FRotator(0, 0, 0);
  ac_cage_pos=CagePos();
  ac_geo_pos=GeoPos();

  for (auto i : array)
    delete i;
  array.Empty();

  for (auto i : str_map)
    delete i.Value;
  str_map.Empty();

  for (auto i : int_map)
    delete i.Value;
  int_map.Empty();

  for (auto i : pos2_map)
    delete i.Value;
  pos2_map.Empty();

  for (auto i : pos3_map)
    delete i.Value;
  pos3_map.Empty();

  str_set.Empty();
  bytes.Empty();  
}


//====================================================================================================
enum BinaryTypes {
  BIN_MAGIC=0,
  BIN_NONE,
  BIN_TRUE,
  BIN_FALSE,
  BIN_INT,
  BIN_FLOAT,
  BIN_DOUBLE,
  BIN_FVECTOR,
  BIN_FROTATOR,
  BIN_CAGE_POS,
  BIN_GEO_POS,
  BIN_EMPTY_MAP,
  BIN_EMPTY_SET,
  BIN_ARRAY_START,
  BIN_STR_MAP_START,
  BIN_INT_MAP_START,
  BIN_POS2_MAP_START,
  BIN_POS3_MAP_START,
  BIN_STR_SET_START,
  BIN_END,
  BIN_STRING, // начиная с этого все поля имеют длинну
  BIN_BYTES,
  BIN_COUNT,  
};

//====================================================================================================
// сохранение
bool Acf::save(const FString &filename) {
  std::ofstream file(*filename, std::ios::binary);
  file.write((char*)&MAGIC, 4);
  writeInt(file, VERSION);
  TMap<FString, int> keys;
  try {
   save(file, keys);
  } catch (...) {
    return false;
  }
  return true;
}

//====================================================================================================
void Acf::save(std::ostream &file, TMap<FString, int> &keys) {     
  switch (type) {
    case NONE:
      writeChar(file, BIN_NONE);
      break;

    case BOOL:
      writeChar(file, ac_bool ? BIN_TRUE : BIN_FALSE);
      break;

    case INT:
      writeChar(file, BIN_INT);
      writeInt(file, ac_int);
      break;

    case FLOAT:
      if ((double)(float)ac_float==ac_float) {
        writeChar(file, BIN_FLOAT);
        writeFloat(file, ac_float);
      } else {
        writeChar(file, BIN_DOUBLE);
        writeDouble(file, ac_float);
      }
      break;

    case STRING:
      writeChar(file, BIN_STRING);
      writeKey(file, ac_str, keys);
      break;

    case FVECTOR:
      writeChar(file, BIN_FVECTOR);
      writeFloat(file, ac_fvector.X);
      writeFloat(file, ac_fvector.Y);
      writeFloat(file, ac_fvector.Z);
      break;
      
    case FROTATOR:
      writeChar(file, BIN_FROTATOR);
      writeFloat(file, ac_frotator.Roll);
      writeFloat(file, ac_frotator.Pitch);
      writeFloat(file, ac_frotator.Yaw);
      break;

    case CAGE_POS:
      writeChar(file, BIN_CAGE_POS);
      writeInt(file, ac_cage_pos.x);
      writeInt(file, ac_cage_pos.y);
      writeInt(file, ac_cage_pos.z);
      break;
  
    case GEO_POS:
      writeChar(file, BIN_GEO_POS);
      writeDouble(file, ac_geo_pos.latitude);
      writeDouble(file, ac_geo_pos.longitude);
      writeDouble(file, ac_geo_pos.elevation);
      break;

    case ARRAY:
      writeChar(file, BIN_ARRAY_START);
      for (auto item : array)
        item->save(file, keys);
      writeChar(file, BIN_END);
      break;

    case EMPTY_MAP:
      writeChar(file, BIN_EMPTY_MAP);
      break;

    case STR_MAP:
      writeChar(file, BIN_STR_MAP_START);
      for (auto i : str_map) {
        writeKey(file, i.Key, keys);
        i.Value->save(file, keys);
      }
      writeChar(file, BIN_END);
      break;

    case INT_MAP:
      writeChar(file, BIN_INT_MAP_START);
      for (auto i : int_map) {
        writeInt(file, i.Key);
        i.Value->save(file, keys);
      }
      writeChar(file, BIN_END);
      break;

    case POS2_MAP:
      writeChar(file, BIN_POS2_MAP_START);
      for (auto i : pos2_map) {
        writeInt(file, i.Key.x);
        writeInt(file, i.Key.y);
        i.Value->save(file, keys);
      }
      writeChar(file, BIN_END);
      break;

    case POS3_MAP:
      writeChar(file, BIN_POS3_MAP_START);
      for (auto i : pos3_map) {
        writeInt(file, i.Key.x);
        writeInt(file, i.Key.y);
        writeInt(file, i.Key.z);
        i.Value->save(file, keys);
      }
      writeChar(file, BIN_END);
      break;

    case EMPTY_SET:
      writeChar(file, BIN_EMPTY_MAP);
      break;

    case STR_SET:
      writeChar(file, BIN_STR_SET_START);
      for (auto &str : str_set)
        writeKey(file, str, keys);
      writeChar(file, BIN_END);
      break;

    case BYTES:
      writeChar(file, BIN_BYTES);
      writeBytes(file, bytes);
      break;
  }
}

//====================================================================================================
// переводим кол-во байт в марку
static long long bytes2mark(int bytes) {
  return ((0x80ull>>(bytes-1))<<(8*(bytes-1)));  
}

//====================================================================================================
void Acf::writeChar(std::ostream &file, char value) {
  file<<value;
}

//====================================================================================================
void Acf::writeInt(std::ostream &file, long long value) {
  if (value>=0)
    value*=2;
  else
    value=-value*2+1;

  if (value<bytes2mark(4))
    if (value<bytes2mark(2))
      if (value<bytes2mark(1))
        file  << (BYTE)(value|bytes2mark(1));
      else {
        value|=bytes2mark(2);
        file  << (BYTE)(value>>8) 
              << (BYTE)value;
      }
    else
      if (value<bytes2mark(3)) {
        value|=bytes2mark(3);
        file  << (BYTE)(value>>16) 
              << (BYTE)(value>>8) 
              << (BYTE)value;
      } else {
        value|=bytes2mark(4);
        file  << (BYTE)(value>>24) 
              << (BYTE)(value>>16) 
              << (BYTE)(value>>8) 
              << (BYTE)value;
      }
  else
    if (value<bytes2mark(6))
      if (value<bytes2mark(5)) {
        value|=bytes2mark(5);
        file  << (BYTE)(value>>32) 
              << (BYTE)(value>>24) 
              << (BYTE)(value>>16) 
              << (BYTE)(value>>8) 
              << (BYTE)value;
      } else {
        value|=bytes2mark(6);
        file  << (BYTE)(value>>40) 
              << (BYTE)(value>>32) 
              << (BYTE)(value>>24) 
              << (BYTE)(value>>16) 
              << (BYTE)(value>>8) 
              << (BYTE)value;
      }
    else
      if (value<bytes2mark(7)) {
        value|=bytes2mark(7);
        file  << (BYTE)(value>>48)
              << (BYTE)(value>>40)
              << (BYTE)(value>>32)
              << (BYTE)(value>>24)
              << (BYTE)(value>>16)
              << (BYTE)(value>>8)
              << (BYTE)(value);
      } else {
        if (value<bytes2mark(8)) {
          value|=bytes2mark(8);
          file<< (BYTE)(value>>56)
              << (BYTE)(value>>48)
              << (BYTE)(value>>40)
              << (BYTE)(value>>32)
              << (BYTE)(value>>24)
              << (BYTE)(value>>16)
              << (BYTE)(value>>8)
              << (BYTE)value;
        } else {
          file<< (BYTE)(value>>56)
              << (BYTE)(value>>48)
              << (BYTE)(value>>40)
              << (BYTE)(value>>32)
              << (BYTE)(value>>24)
              << (BYTE)(value>>16)
              << (BYTE)(value>>8)
              << (BYTE)value;
        }
      }  
}

//====================================================================================================
void Acf::writeString(std::ostream &file, const FString &value) {
  FTCHARToUTF8 utf8(*value);

  writeInt(file, utf8.Length());
  if (utf8.Length())
    file.write(utf8.Get(), utf8.Length());
}

//====================================================================================================
void Acf::writeKey(std::ostream &file, const FString &key, TMap<FString, int> &keys) {
  if (auto exist=keys.Find(key))
    writeInt(file, *exist);
  else {
    int id = keys.Num();
    keys.Add(key, id);
    writeInt(file, id);
    writeString(file, key);
  }
}

//====================================================================================================
void Acf::writeFloat(std::ostream &file, float value) {
  file.write((char*)&value, sizeof(value));
}

//====================================================================================================
void Acf::writeDouble(std::ostream &file, double value) {
  file.write((char*)&value, sizeof(value));
}

//====================================================================================================
void Acf::writeBytes(std::ostream &file, const TArray<BYTE> &value) {
  writeInt(file, value.Num());
  if (value.Num())
    file.write((const char*)value.GetData(), value.Num());
}


//====================================================================================================
// загрузка
bool Acf::load(const FString &filename) {
  empty();
  std::ifstream file(*filename, std::ios::binary);
  int magic;
  file.read((char*)&magic, 4);
  int version;
  version=readInt(file);
  if (magic!=MAGIC)
    return false;
  TMap<int, FString> keys;
  try {
    load(file, version, keys);
  } catch (...) {
    empty();
    return false;
  }
  return true;
}

//====================================================================================================
void Acf::load(std::istream &file, int version, TMap<int, FString> &keys) {
  switch (readChar(file)) {
  case BIN_MAGIC:
    readSkip(file, 3);
    readInt(file);
    break;

  case BIN_NONE:
    type=NONE;
    break;

  case BIN_TRUE:
    type=BOOL;
    ac_bool=true;
    break;

  case BIN_FALSE:
    type=BOOL;
    ac_bool=false;
    break;

  case BIN_INT:
    type=INT;
    ac_int=readInt(file);
    break;

  case BIN_FLOAT:
    type=FLOAT;
    ac_float=readFloat(file);
    break;

  case BIN_DOUBLE:
    type=FLOAT;
    ac_float=readDouble(file);
    break;

  case BIN_FVECTOR:
    type=FVECTOR;
    ac_fvector.X=readFloat(file);
    ac_fvector.Y=readFloat(file);
    ac_fvector.Z=readFloat(file);
    break;

  case BIN_FROTATOR:
    type=FROTATOR;
    ac_frotator.Roll=readFloat(file);
    ac_frotator.Pitch=readFloat(file);
    ac_frotator.Yaw=readFloat(file);
    break;

  case BIN_CAGE_POS:
    type=CAGE_POS;
    ac_cage_pos.x=readInt(file);
    ac_cage_pos.y=readInt(file);
    ac_cage_pos.z=readInt(file);
    break;

  case BIN_GEO_POS:
    type=GEO_POS;
    ac_geo_pos.latitude=readDouble(file);
    ac_geo_pos.longitude=readDouble(file);
    ac_geo_pos.elevation=readDouble(file);
    break;

  case BIN_EMPTY_MAP:
    type=EMPTY_MAP;
    break;

  case BIN_EMPTY_SET:
    type=EMPTY_SET;
    break;

  case BIN_ARRAY_START:
    type=ARRAY;
    while (file.peek()!=BIN_END) {
      array.Add(new Acf(file, version, keys));
      if (file.eof())
        throw AcfException();
    }
    check(readChar(file)==BIN_END);
    break;

  case BIN_STR_MAP_START:
    type=STR_MAP;
    while (file.peek()!=BIN_END) {
      auto key=readKey(file, keys);
      str_map.Add(key, new Acf(file, version, keys));
      if (file.eof())
        throw AcfException();
    }
    check(readChar(file)==BIN_END);
    break;

  case BIN_INT_MAP_START:
    type=INT_MAP;
    while (file.peek()!=BIN_END) {
      auto key=readInt(file);
      int_map.Add(key, new Acf(file, version, keys));
      if (file.eof())
        throw AcfException();
    }
    check(readChar(file)==BIN_END);
    break;

  case BIN_POS2_MAP_START:
    type=POS2_MAP;
    while (file.peek()!=BIN_END) {
      Pos2D key(readInt(file), readInt(file));
      pos2_map.Add(key, new Acf(file, version, keys));
      if (file.eof())
        throw AcfException();
    }
    check(readChar(file)==BIN_END);
    break;

  case BIN_POS3_MAP_START:
    type=POS3_MAP;
    while (file.peek()!=BIN_END) {
      Pos3D key(readInt(file), readInt(file), readInt(file));
      pos3_map.Add(key, new Acf(file, version, keys));
      if (file.eof())
        throw AcfException();
    }
    check(readChar(file)==BIN_END);
    break;

  case BIN_STR_SET_START:
    type=STR_SET;
    while (file.peek()!=BIN_END) {
      auto key=readKey(file, keys);
      str_set.Add(key);
      if (file.eof())
        throw AcfException();
    }
    check(readChar(file)==BIN_END);
    break;

  case BIN_END:
    // TODO: здесь ошибка - такой символ не должен вычитываться!
    break;

  case BIN_STRING:
    type=STRING;
    ac_str=readKey(file, keys);
    break;

  case BIN_BYTES:
    type=BYTES;
    bytes=readBytes(file);
    break;

  default:
    readSkip(file, readInt(file));
  };
}

//====================================================================================================
char Acf::readChar(std::istream &file) {
  char byte=0;
  file.read(&byte, 1);
  return byte;
}

//====================================================================================================
long long Acf::readInt(std::istream &file) {
  BYTE hi_byte=readChar(file);

  for (BYTE _bytes=1; _bytes<=9; _bytes++) {
    BYTE mark=(0x80>>(_bytes-1));
    if (hi_byte>=mark) {
      long long value=hi_byte^mark;
      for (int i=1; i<_bytes; i++)
        value=(value<<8)|readChar(file);

      if (value & 1)
        value=-((value-1)/2);
      else
        value=value/2;

      return value;
    }
  }

  check(false);
  return 0;
}


//====================================================================================================
float Acf::readFloat(std::istream &file) {
  float value=0;
  file.read((char*)&value, sizeof(value));
  return value;
}

//====================================================================================================
double Acf::readDouble(std::istream &file) {
  double value=0;
  file.read((char*)&value, sizeof(value));
  return value;
}

//====================================================================================================
FString Acf::readString(std::istream &file) {
  long long len=readInt(file);
  char *utf8=new char[len + 1];
  file.read(utf8, len);
  utf8[len] = '\0';
  
  FString str = UTF8_TO_TCHAR(utf8);
  delete[] utf8;
  return str;
}

//====================================================================================================
FString Acf::readKey(std::istream &file, TMap<int, FString> &keys) {
  int id=readInt(file);
  if (auto exist=keys.Find(id))
    return *exist;
  auto str=readString(file);
  keys.Add(id, str);
  return str;
}

//====================================================================================================
TArray<BYTE> Acf::readBytes(std::istream &file) {
  long long len=readInt(file);
  TArray<BYTE> _bytes;
  _bytes.AddZeroed(len);
  file.read((char*)_bytes.GetData(), len);
  return _bytes;
}

//====================================================================================================
void Acf::readSkip(std::istream &file, long long count) {
  file.ignore(count);
}
