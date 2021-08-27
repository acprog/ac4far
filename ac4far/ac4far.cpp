// AC4FAR (c) Alexander Semenov 2020-2021
#include "pch.h"
#include <map>
#include <string>
#include <Acf.h>
#include <plugin.hpp>
#include "acf_panel.h"

// {8FDEF24A-EBE8-46BE-AF9A-7A017C90C3F9}
static const GUID guid_plugin = 
{ 0x8fdef24a, 0xebe8, 0x46be, { 0xaf, 0x9a, 0x7a, 0x1, 0x7c, 0x90, 0xc3, 0xf9 } };

// {5C7B86CA-F547-4841-B3D1-5253656F027D}
static const GUID guid_mkdir = 
{ 0x5c7b86ca, 0xf547, 0x4841, { 0xb3, 0xd1, 0x52, 0x53, 0x65, 0x6f, 0x2, 0x7d } };

// {AACBA185-E183-4BF7-940C-55173AF1C2B9}
static const GUID guid_save = 
{ 0xaacba185, 0xe183, 0x4bf7, { 0x94, 0xc, 0x55, 0x17, 0x3a, 0xf1, 0xc2, 0xb9 } };

// {DA52573E-7833-4209-A692-30CC5924EDCD}
static const GUID guid_delete = 
{ 0xda52573e, 0x7833, 0x4209, { 0xa6, 0x92, 0x30, 0xcc, 0x59, 0x24, 0xed, 0xcd } };

// {1317DFA6-7358-4444-B44C-69D42FB4BF69}
static const GUID guid_getfiles = 
{ 0x1317dfa6, 0x7358, 0x4444, { 0xb4, 0x4c, 0x69, 0xd4, 0x2f, 0xb4, 0xbf, 0x69 } };

// {A38A94D4-58D3-49AD-A0E5-125782974411}
static const GUID guid_put = 
{ 0xa38a94d4, 0x58d3, 0x49ad, { 0xa0, 0xe5, 0x12, 0x57, 0x82, 0x97, 0x44, 0x11 } };

#define MAX_SHOW_REMOVE 10

static struct PluginStartupInfo far_api;
static std::map<std::wstring, AcfContainer*> containers;

extern "C" {
//  int __cdecl sscanf_wrapper(char const* const str, char const* const format, va_list args) {
//    return vsscanf(str, format, args);
//  }


  //========================================================================================================================================================================================
  // Far Manager вызывает функцию GetGlobalInfoW в первую очередь, для получения основной информации о плагине.Функция вызывается один раз.
  __declspec(dllexport) void WINAPI GetGlobalInfoW(struct GlobalInfo *info)
  {
    info->StructSize = sizeof(*info);
    info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 2927, VS_RELEASE);   //минимально необходимая версия Far Manager, смотрите изменения в API
    info->Version = MAKEFARVERSION(1, 0, 0, 0, VS_ALPHA);   //текущая версия плагина 3.0.0.21, релиз-кандидат
    info->Guid = guid_plugin;
    info->Title = L"AC4Far";
    info->Description = L"AC tools for Far Manager";
    info->Author = L"Alexander Semenov";
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция SetStartupInfoW вызывается один раз, после загрузки DLL - модуля в память.Far Manager передаёт плагину информацию, необходимую для дальнейшей работы.
  //Эта функция вызывается сразу же после вызова функции GetGlobalInfoW.
  //Указатель Info действителен только в области видимости данной функции(до выхода из функции), так что структура должна копироваться во внутреннюю переменную плагина для дальнейшего использования :
  __declspec(dllexport) void WINAPI SetStartupInfoW(const struct PluginStartupInfo *info)
  {
    if (!info || info->StructSize < sizeof(*info))
      return;

    far_api = *info;
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Функция GetPluginInfoW вызывается Far Manager для получения дополнительной информации о плагине.
  __declspec(dllexport) void WINAPI GetPluginInfoW(struct PluginInfo *info)
  {
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Функция ExitFARW вызывается перед выходом из Far Manager.В этой функции плагин может удалить все используемые ресурсы, например, ранее запрошенную память.
  __declspec(dllexport) void WINAPI ExitFARW(const struct ExitInfo *Info)
  {
    for (auto container : containers) {
//      container.second->save();
      delete container.second;
    }
    containers.clear();
  }

  //========================================================================================================================================================================================
  // Far Manager вызывает функцию AnalyseW для открытия панели плагина, эмулирующего файловую систему на базе переданного этой функции файла (например, архива).
  __declspec(dllexport) HANDLE WINAPI AnalyseW(const struct AnalyseInfo *info)
  {    
    if (!info || info->StructSize < sizeof(*info))
      return nullptr;

    if (info->FileName == nullptr || !info->BufferSize)
      return nullptr;

    if (containers.find(info->FileName) != containers.end())
      return containers[info->FileName];
    
    if (info->BufferSize == 4 && *(UINT32*)info->Buffer == 'BCA\0') {
      AcfContainer *container = new AcfContainer(info->FileName, new Acf());
      containers[container->file_name] = container;
      return container;
    }

    try {
      std::string name = FString::wcharToUtf8(info->FileName);
      Acf *acf = new Acf();
      if (!acf->load(name.c_str())) {
        delete acf;
        return nullptr;
      }
      AcfContainer *container= new AcfContainer(info->FileName, acf);
      containers[container->file_name] = container;
      return container;
    }
    catch (...) {
    }

    return nullptr;
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Функция OpenW вызывается Far Manager'ом для запуска плагина.
  __declspec(dllexport) HANDLE WINAPI OpenW(const struct OpenInfo *info)
  {
    if (!info || info->StructSize < sizeof(*info))
      return nullptr;

    if (info->OpenFrom != OPEN_ANALYSE)
      return nullptr;

    OpenAnalyseInfo *analys = (OpenAnalyseInfo*)info->Data;
    AcfContainer *container = (AcfContainer*)analys->Handle;
    if (!container)
      return nullptr;

    return new AcfPanel(*container);
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция CloseAnalyseW вызывается Far Manager для передачи плагину данных, которые плагин может удалить.
  __declspec(dllexport) void WINAPI CloseAnalyseW(const struct CloseAnalyseInfo *info)
  {
    if (!info || info->StructSize < sizeof(*info))
      return;

    //MessageBox(NULL, L"CloseAnalyseW", L"", MB_OK);
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция ClosePanelW вызывается Far Manager перед закрытием открытой панели плагина.
  __declspec(dllexport) void WINAPI ClosePanelW(const struct ClosePanelInfo *info)
  {
    if (!info || info->StructSize < sizeof(*info))
      return;

    if (info->hPanel) {
      AcfPanel *panel = (AcfPanel*)info->hPanel;
      AcfContainer *container = &panel->container;      

      if (container->panels.size() == 1) {
        if (container->changed) {
          const wchar_t *msg[] = {
            L"Сохранить изменения в ACF?",
            //L"",
            container->file_name.c_str()
          };

//          if (far_api.Message(&guid_plugin, &guid_save, FMSG_MB_YESNO | FMSG_WARNING, NULL, msg, 2, 0) == 0)
//            container->save();
        }
      }

      delete panel;
      if (container->panels.size() == 0) {
        containers.erase(container->file_name);
        delete container;
      }
    }
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция GetOpenPanelInfoW вызывается Far Manager для получения информации об открываемой панели плагина.
  __declspec(dllexport) void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *info)
  {
    if (!info || info->StructSize < sizeof(*info))
      return;
    if (!info->hPanel)
      return;

    AcfPanel *panel = (AcfPanel*)info->hPanel;
    AcfContainer &container = panel->container;
    info->StructSize = sizeof(*info);
    info->Flags = OPIF_ADDDOTS | OPIF_SHOWPRESERVECASE; // OPIF_DISABLEHIGHLIGHTING
    info->HostFile = container.file_name.c_str();
    info->CurDir = panel->cur_dir.length() > 0 ? panel->cur_dir.c_str() : nullptr;
    info->Format = container .file_name.c_str();
    info->PanelTitle = panel->title.c_str();
    info->InfoLines = nullptr;
    info->InfoLinesNumber = 0;
    // Адрес массива указателей на имена файлов описаний.Far Manager попытается прочитать эти файлы(функцией GetFilesW) при показе описаний и обновить их после обработки файлов, если флаг PPIF_PROCESSDESCR в структуре PluginPanelItem был установлен.В зависимости от типа плагина, обработка описаний может занять значительное время.Если Вам это не нужно, установите DescrFiles в NULL.
    info->DescrFiles = nullptr;
    info->DescrFilesNumber = 0;
    info->PanelModesArray = nullptr;
    info->PanelModesNumber = 0;
    // Режим просмотра, который будет установлен после создания панели плагина. Он должен быть в формате '0'+<номер режима просмотра>. Например, '1' или 0x31 будут устанавливать Краткий режим просмотра. Если вы не хотите менять режим просмотра панели после запуска плагина, установите StartPanelMode в 0.
    info->StartPanelMode = '6';
    info->StartSortMode = SM_DEFAULT;
    info->StartSortOrder = 0;
    info->KeyBar = nullptr;
    //Адрес текстовой строки, оканчивающейся нулём, описывающей текущее состояние панели плагина.Эта строка передаётся в OpenW, когда плагин активирован командой ссылки на папку.Например, FTP клиент может поместить сюда текущий хост, логин и пароль.Нет необходимости сохранять здесь текущий каталог, потому что он будет восстановлен самим Far Manager'ом.
    //		Если вы не нуждаетесь в такой дополнительной информации для обработки ссылки на папку, установите данную строку в NULL.
    info->ShortcutData = nullptr;
    info->FreeSize = 0;
  }


  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция DeleteFilesW вызывается для удаления файлов из эмулируемой файловой системы
  //(Far Manager плагину : "этот (эти) файлы с твоей панели нужно удалить").
  __declspec(dllexport) intptr_t WINAPI DeleteFilesW(const struct DeleteFilesInfo *info)
  {
    return 0;
    /*
    if (!info->hPanel)
      return 0;

    AcfPanel *panel = (AcfPanel*)info->hPanel;    

    if ((info->OpMode & OPM_SILENT) == 0) {
      const wchar_t *msg[MAX_SHOW_REMOVE + 2] = { NULL };
      msg[0] = L"Удалить эти параметры?";
      size_t i = 0;
      for (; i < info->ItemsNumber; i++)
        if (i < MAX_SHOW_REMOVE)
          msg[i + 1] = info->PanelItem[i].FileName;
        else {
          msg[i + 1] = L"...";
          break;
        }

        if (far_api.Message(&guid_plugin, &guid_delete, FMSG_MB_YESNO | FMSG_WARNING, NULL, msg, i + 1, 0) != 0)
          return 0;
    }

    for (size_t i = 0; i < info->ItemsNumber; i++)
      if (!panel->container.remove((AcfItem*)info->PanelItem[i].UserData.Data))
        return 0;

    return 1;
    */
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция SetDirectoryW вызывается Far Manager'ом для смены каталога в эмулируемой файловой системе.
  __declspec(dllexport) intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo *info)
  {
    if (!info || info->StructSize < sizeof(*info))
      return 0;
    if (!info->hPanel)
      return 0;

    AcfPanel *panel = (AcfPanel*)info->hPanel;
    if (wcschr(info->Dir, L'\\'))
      if (!info->UserData.Data)        
        return 0; // по имени нельзя однозначно определить путь, нужно патчить FAR

    if (wcscmp(info->Dir, L"..") == 0)
      return panel->changeDir(nullptr) ? 1 : 0;
    return panel->changeDir((AcfItem*)info->UserData.Data) ? 1 : 0;
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Функция GetFindDataW вызывается для получения списка файлов из текущего каталога эмулируемой файловой системы
  // (Far Manager плагину : "дай-ка я взгляну на списочек твоих файлов, память сам выдели :-)").
  __declspec(dllexport) intptr_t WINAPI GetFindDataW(struct GetFindDataInfo *info)
  {
    if (info->StructSize < sizeof(*info))
      return 0;

    if (!info->hPanel)
      return 0;

//    if (info->OpMode & OPM_FIND)
      //return 0; //Если вы не хотите, чтобы ваш плагин использовался во время "поиска в архивах" ("[x] Искать в архивах" в диалоге поиска), то возвращайте 0 в случае, если OpMode содержит флаг OPM_FIND.

    AcfPanel *panel = (AcfPanel*)info->hPanel;
    const std::vector<AcfItem*> &items = panel->folder ? panel->folder->items : panel->container.root->items;
    info->PanelItem = new PluginPanelItem[items.size()];
    for (size_t i = 0; i < items.size(); i++)
      info->PanelItem[i] = *(PluginPanelItem*)items[i];
    info->ItemsNumber = items.size();

    return 1;
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Функция FreeFindDataW должна освободить память, выделенную функцией GetFindDataW
  //(Far Manager плагину : "я тут списочек запрашивал, так вот, он мне более не нужен, освободи память").
  __declspec(dllexport) void WINAPI FreeFindDataW(const struct FreeFindDataInfo *info)
  {
    if (info->StructSize < sizeof(*info))
      return;

    if (info->PanelItem)
      delete[] info->PanelItem;
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция MakeDirectoryW вызывается Far Manager для создания нового каталога в эмулируемой файловой системе.
  __declspec(dllexport) intptr_t WINAPI MakeDirectoryW(struct MakeDirectoryInfo *info) {
    return 0;
    /*
    if (info->StructSize < sizeof(*info))
      return 0;

    if (!info->hPanel)
      return 0;

    AcfPanel *panel = (AcfPanel*)info->hPanel;
    static wchar_t buffer[1024] = { 0 };
    if (!far_api.InputBox(&guid_plugin, &guid_mkdir, L"Имя новой группы:", nullptr, nullptr, nullptr, buffer, 1024, nullptr, FIB_NONE))
      return 0;

    info->Name = buffer;
    panel->container.mkdir(panel->folder, info->Name);
    return 1;
    */
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция GetFilesW вызывается для получения файлов из эмулируемой файловой системы
  //(Far Manager плагину : "хочу этот/эти файл(ы) с твоей панели, место назначения указано").
  __declspec(dllexport) intptr_t WINAPI GetFilesW(struct GetFilesInfo *info)
  {
    return 0;
    /*
    if (info->StructSize < sizeof(*info))
      return 0;

    if (!info->hPanel)    
      return 0;

    if (info->OpMode & (OPM_COMMANDS))
      return 0;

    AcfPanel *panel = (AcfPanel*)info->hPanel;

    if ((info->OpMode & OPM_SILENT) == 0) {
      const size_t len = 0x10000;
      static wchar_t buffer[len] = { 0 };
      wcsncpy_s(buffer, info->DestPath, len);
      if (!far_api.InputBox(&guid_plugin, &guid_getfiles, L"Скопировать выбранные параметры в:", nullptr, nullptr, buffer, buffer, len, nullptr, FIB_BUTTONS | FIB_EDITPATH))
        return 0;
    }
    
    edited_item = nullptr;
    for (size_t i = 0; i < info->ItemsNumber; i++) {
      AcfItem *item = (AcfItem*)info->PanelItem[i].UserData.Data;
      bool done =item->get(info->DestPath);
      if (!done)
        return 0;
      if (info->OpMode & OPM_EDIT)
        edited_item = item;
      info->PanelItem[i].Flags &= ~PPIF_SELECTED;
      item->Flags &= ~PPIF_SELECTED;
      if (info->Move) {
        panel->container.remove(item);
        panel->container.changed = true;
      }
    }

    return 1;
    */
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция PutFilesW вызывается Far Manager для перемещения файлов на панель эмулируемой файловой системы.
  //(Far Manager плагину : "этот/эти файл(ы) для тебя, можешь поместить на свою панель").
  intptr_t WINAPI PutFilesW(const struct PutFilesInfo *info)
  {
    return 0;
    /*
    if (info->StructSize < sizeof(*info))
      return 0;

    if (!info->hPanel)
      return 0;

    AcfPanel *panel = (AcfPanel*)info->hPanel;
    if (!panel->folder){
      const wchar_t *msg[] = {
        L"",
        L"Параметры можно копировать только в группу",        
      };
      far_api.Message(&guid_plugin, &guid_put, FMSG_MB_OK | FMSG_WARNING, NULL, msg, 2, 0);
      return 0; // не копируем в root
    }

    std::wstring path = info->SrcPath;
    for (size_t i = 0; i < info->ItemsNumber; i++) {
      if (info->PanelItem[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue; // не копируем каталоги

      bool done = panel->folder->put(info->SrcPath, info->PanelItem[i].FileName, info->Move != 0);
      if (!done)
        return 0;

      if (info->OpMode & OPM_EDIT && edited_item) {
        panel->container.remove(edited_item);
        edited_item = nullptr;
      }

      panel->container.changed = true;
      info->PanelItem[i].Flags &= ~PPIF_SELECTED;
    }

    return 1;
    */
  }

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция ProcessPanelEventW сообщает плагину о разных событиях панели и позволяет обработать некоторые из них.
#if 0
  __declspec(dllexport) intptr_t WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *info)
  {
    if (info->StructSize < sizeof(*info))
      return 0;

    if (!info->hPanel)
      return 0;

    switch (info->Event) {
    case FE_CHANGEVIEWMODE:
    case FE_REDRAW:
    case FE_IDLE: // Посылается с интервалом в несколько секунд. Плагин может использовать это событие, чтобы вызвать обновление панели и её перерисовку, если это необходимо
    case FE_CLOSE: // Верните в ProcessPanelEventW FALSE для закрытия панели или TRUE для отмены этого.
    case FE_BREAK:
    case FE_COMMAND:
    case FE_KILLFOCUS:
    case FE_GOTFOCUS:
    case FE_CHANGESORTPARAMS: // Сменились параметры сортировки панели.
      return false;
    }
    return false;
  }
#endif

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция ProcessHostFileW вызывается, чтобы выполнить архивные команды Far Manager. Рекомендуется поместить сюда дополнительные операции, которые обрабатывают файл, на основании которого плагин эмулирует файловую систему.
  //intptr_t WINAPI ProcessHostFileW(
  //	const struct ProcessHostFileInfo *Info
  //);

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция SetFindListW предназначена для переноса найденных объектов из диалога Поиска файлов в эмулируемую файловую систему.Физически файлы не должны копироваться или изменяться.
  //intptr_t WINAPI SetFindListW(const struct SetFindListInfo *Info)
  //{
  //	TmpPanel *Panel = (TmpPanel *)Info->hPanel;
  //	return Panel->SetFindList(Info->PanelItem, Info->ItemsNumber);
  //}
  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Плагин может экспортировать функцию CompareW, чтобы перекрыть используемый по умолчанию алгоритм сортировки файлов.
  //intptr_t WINAPI CompareW(const struct CompareInfo *Info)
  //{
  //	return ((Plist *)Info->hPanel)->Compare(Info->Item1, Info->Item2, Info->Mode);
  //}

  //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //Функция ProcessPanelInputW позволяет перекрыть стандартную обработку клавиш и мыши(сейчас не задействовано) на панели плагина.
  //intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
  //{
  //	return ((Plist *)Info->hPanel)->ProcessKey(&Info->Rec);
  //}
}