// AC4FAR (c) Alexander Semenov 2020-2021
#pragma once
#include "pch.h"
#include <acf.h>
#include <sstream>
#include <iomanip>

class AcfItem;
static AcfItem *edited_item = nullptr; // запоминается чтобы не множить сущности TODO: элемент не должен удаляться во время редактирования!

//=======================================================================================================================================
// один элемент в панели
//=======================================================================================================================================
class AcfItem : public PluginPanelItem {
public:
  //------------------------------------------------------------------------------------------------------------------------------------------
  std::wstring name;  // содержит строки, т.к. в PluginPanelItem будет просто указатель на них
  std::wstring description;
  AcfItem *parent;
  Acf *acf;
  std::vector<AcfItem*> items;

  //------------------------------------------------------------------------------------------------------------------------------------------
  AcfItem(const PluginPanelItem &defitem, AcfItem *_parent, const std::wstring &_name, Acf *_acf)
    :name(_name)
    ,parent(_parent)
    ,acf(_acf)
  {
    *((PluginPanelItem*)this) = defitem;
    UserData.Data = this;
    AlternateFileName = FileName = name.c_str();

    description=getDescription();
    Description = description.c_str();

    if (isFolder()) {
      FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
      if (acf->isArray()) {
        int id=0;
        for (auto i : acf->asArray()) {
          std::wstringstream item_name;
          item_name<<"["<<id<<"]";
          items.push_back(new AcfItem(defitem, this, item_name.str(), i));
          id++;
        }
      }
      if (acf->isIntMap()) {
        for (auto i : acf->asIntMap()) {
          std::wstringstream item_name;
          item_name<<i.first;
          items.push_back(new AcfItem(defitem, this, item_name.str(), i.second));
        }
      }
      if (acf->isStrMap()) {
        for (auto i : acf->asStrMap()) {
          std::wstringstream item_name;
          item_name<<i.first;
          items.push_back(new AcfItem(defitem, this, item_name.str(), i.second));
        }
      }
      if (acf->isPos2Map()) {
        for (auto i : acf->asPos2Map()) {
          std::wstringstream item_name;
          item_name<<"Pos2D("<<i.first.x<<", "<<i.first.y<<")";
          items.push_back(new AcfItem(defitem, this, item_name.str(), i.second));
        }
      }
      if (acf->isPos3Map()) {
        for (auto i : acf->asPos3Map()) {
          std::wstringstream item_name;
          item_name<<"Pos3D("<<i.first.x<<", "<<i.first.y<<", "<<i.first.z<<")";
          items.push_back(new AcfItem(defitem, this, item_name.str(), i.second));
        }
      }
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  ~AcfItem() { // не виртуальный, чтобы не было vptr вначале
    if (edited_item == this)
      edited_item = nullptr;
    for (auto item : items)
      delete item;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  bool isFolder() {
    if (acf->isNone())
      return false;    
    return acf->isArray() || acf->isIntMap() || acf->isStrMap() || acf->isPos2Map() || acf->isPos3Map();
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  std::wstring getDescription() {
    std::wstringstream desc;

    if (acf->isNone())
      desc<<"none";
    else if (acf->isBool())
      desc<<(*acf ? "true" : "false");
    else if (acf->isInt())
      desc<<(long long)*acf;
    else if (acf->isFloat())
      desc<<(double)*acf;
    else if (acf->isString())
      desc<<"\""<<((const FString&)*acf)<<"\"";
    else if (acf->isFVector()) {
      FVector v(*acf);
      desc<<"FVector("<<v.X<<", "<<v.Y<<", "<<v.Z<<")";
    } else if (acf->isFRotator()) {
      FRotator r(*acf);
      desc<<"FRotator("<<r.Roll<<", "<<r.Pitch<<", "<<r.Yaw<<")";
    } else if (acf->isPos2D()) {
      Pos2D pos(*acf);
      desc<<"Pos2D("<<pos.x<<", "<<pos.y<<")";
    } else if (acf->isPos3D()) {
      Pos3D pos(*acf);
      desc<<"Pos3D("<<pos.x<<", "<<pos.y<<", "<<pos.z<<")";
    } else if (acf->isGeoPos()) {
      GeoPos pos(*acf);
      desc<<"GeoPos("<<pos.latitude<<", "<<pos.longitude<<", "<<pos.elevation<<")";
    } else if (acf->isStrSet()) {
      desc << "{";
      for (auto &i : acf->asStrSet())
        desc<<"\""<<i<<"\", ";
      desc << "}";
    } else if (acf->isIntSet()) {
      desc << "{";
      for (auto i : acf->asIntSet())
        desc<<""<<i<<", ";
      desc << "}";
    } else if (acf->isBytes()) {
      for (auto i : acf->asBytes())
        desc<< "0x" << std::uppercase << std::setfill(L'0') << std::setw(4) << std::hex << i << ", ";
    } else if (acf->isArray()) {
      desc<<"[.."<<acf->asArray().Num()<<"]";
    } else if (acf->isMap()) {
      desc<<"{}";
    } else if (acf->isIntMap()) {
      desc<<"{int: .."<<acf->asIntMap().Num()<<"}";
    } else if (acf->isStrMap()) {
      desc<<"{str: .."<<acf->asStrMap().Num()<<"}";
    } else if (acf->isPos2Map()) {
      desc<<"{pos2: .."<<acf->asPos2Map().Num()<<"}";
    } else if (acf->isPos3Map()) {
      desc<<"{pos3: .."<<acf->asPos3Map().Num()<<"}";
    }
    
    return desc.str();
  }
};