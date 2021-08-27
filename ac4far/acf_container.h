// AC4FAR (c) Alexander Semenov 2020-2021
#pragma once
#include "pch.h"
#include <vector>
#include "acf_item.h"

//=============================================================================================================
// бызовый класс для панели
//=============================================================================================================
class AcfPanelBase {
public:
  AcfItem * folder = nullptr;

  virtual ~AcfPanelBase() {}
};


//=============================================================================================================
// контейнер, содержащий ACF
//=============================================================================================================
class AcfContainer {
public:
  std::wstring file_name;
  AcfItem *root;
  std::vector<AcfPanelBase*> panels;
  PluginPanelItem defitem;
  bool changed = false;
  int refcount = 0;

  //---------------------------------------------------------------------------------------------------------
  AcfContainer(const wchar_t *_file_name, Acf *acf)
    :file_name(_file_name)
  {
    memset(&defitem, 0, sizeof(defitem));
    HANDLE hFile = CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
      GetFileTime(hFile, &defitem.CreationTime, &defitem.LastAccessTime, &defitem.LastWriteTime);
      defitem.ChangeTime = defitem.LastWriteTime;
      CloseHandle(hFile);
    }

    defitem.Owner = nullptr;
    defitem.Flags = PPIF_NONE;
    defitem.FileAttributes = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_VIRTUAL;
    defitem.NumberOfLinks = 0;
    defitem.CRC32 = 0;

    root=new AcfItem(defitem, nullptr, TEXT(""), acf);
  }

  //---------------------------------------------------------------------------------------------------------
  virtual ~AcfContainer() {
    delete root->acf;
    delete root;
  }
/*
  //---------------------------------------------------------------------------------------------------------
  bool remove(const AcfItem *item) {
    auto &items = item->parent ? item->parent->items : this->items;
    items.erase(std::remove(items.begin(), items.end(), item), items.end());
    delete item;

    for (auto panel : panels) {
      AcfItem *folder = panel->folder;
      while (folder) {
        if (folder->parent == item)
          panel->folder = item->parent;
        folder = folder->parent;
      }
      // TODO: обновить панель! PanelControl(FCTL_REDRAWPANEL)
    }

    changed = true;
    return true;
  }

  //---------------------------------------------------------------------------------------------------------
  void mkdir(AcfItem *folder, const wchar_t *name) {
    changed = true;
    if (folder)
      folder->items.push_back(new AcfItem(defitem, folder, name));
    else
      items.push_back(new AcfItem(defitem, folder, name));

    // TODO: обновить все панели!! PanelControl(FCTL_REDRAWPANEL)
  }


  //---------------------------------------------------------------------------------------------------------
  void save() {
    try {
      std::wstring tmpname = file_name + L".__tmp__";

      {
        AcfWriter writer(wcharToUtf8(tmpname.c_str()).c_str(), ACF_WRITE_BINARY);
        for (auto item : items)
          item->write(writer);
      }

      ReplaceFile(file_name.c_str(), tmpname.c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, 0, 0);
    }
    catch (...) {
    }
  }
*/
};
