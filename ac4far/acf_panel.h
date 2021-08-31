// AC4FAR (c) Alexander Semenov 2020-2021
#pragma once
#include "pch.h"
#include "acf_item.h"
#include "acf_container.h"

//=============================================================================================================
// панель, отображающая ACF
//=============================================================================================================
class AcfPanel : public AcfPanelBase {
public:
  std::wstring cur_dir;
  std::wstring title;
  AcfItem *prevFolder;
  AcfContainer &container;
 
	//---------------------------------------------------------------------------------------------------------
	AcfPanel(AcfContainer &_container)
    :container(_container)
    ,title(L"ACF:"+ _container.file_name)
  {    
    container.panels.push_back(this);
	}

	//---------------------------------------------------------------------------------------------------------
	virtual ~AcfPanel() {
    container.panels.erase(std::remove(container.panels.begin(), container.panels.end(), this), container.panels.end());
	}

  //---------------------------------------------------------------------------------------------------------
  bool changeDir(AcfItem *item) {
    if (item) {
      if (!item->isFolder())
        return false;
      folder = item;
      cur_dir += L"/";
      cur_dir += folder->name;
        title = L"ACF:" + container.file_name+cur_dir;
      return true;
    }
   
    if (!folder || !folder->parent) {
      folder = nullptr;
      cur_dir = L"";
      return true;
    }

    cur_dir.erase(cur_dir.length() - folder->name.length()-1, folder->name.length()+1);
//    cur_dir[cur_dir.length() - folder->name.length() - 1] = L'\0';
    folder=folder->parent;
    title = L"ACF:" + container.file_name+cur_dir;
    return true;
  }
};
