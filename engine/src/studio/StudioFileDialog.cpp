#include "studio/StudioFileDialog.h"
#include <iostream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#include <algorithm>
#include <cwchar>
#include <vector>
#endif

namespace subspace {
namespace {
#ifdef _WIN32
constexpr wchar_t kBlueprintFilter[] =
    L"Subspace blueprints (*.subspace_ship)\0*.subspace_ship\0All files (*.*)\0*.*\0";

bool Choose(const std::filesystem::path& directory,const std::filesystem::path& initial,
            bool save,std::filesystem::path& selected,std::string& error){
    selected.clear();error.clear();
    std::vector<wchar_t> buffer(32768,L'\0');
    const auto initialFile=initial.filename().wstring();
    const auto count=std::min(initialFile.size(),buffer.size()-1);
    std::copy_n(initialFile.begin(),count,buffer.begin());
    const auto dir=directory.wstring();
    OPENFILENAMEW request{};
    request.lStructSize=sizeof(request);
    request.hwndOwner=GetActiveWindow();
    request.lpstrFilter=kBlueprintFilter;
    request.lpstrFile=buffer.data();
    request.nMaxFile=static_cast<DWORD>(buffer.size());
    request.lpstrInitialDir=dir.empty()?nullptr:dir.c_str();
    request.lpstrDefExt=L"subspace_ship";
    request.lpstrTitle=save?L"Save Subspace ship as":L"Open Subspace ship";
    request.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|
                  (save?0u:OFN_FILEMUSTEXIST);
    const BOOL chosen=save?GetSaveFileNameW(&request):GetOpenFileNameW(&request);
    if(!chosen){
        const DWORD code=CommDlgExtendedError();
        if(code)error="Native file dialog failed (code "+std::to_string(code)+")";
        return false;
    }
    selected=std::filesystem::path(buffer.data());
    return true;
}
#endif
}

bool StudioFileDialog::ChooseOpen(const std::filesystem::path& directory,
                                  std::filesystem::path& selected,std::string& error){
#ifdef _WIN32
    return Choose(directory,{},false,selected,error);
#else
    (void)directory;selected.clear();error="Native Studio dialogs require Windows";return false;
#endif
}
bool StudioFileDialog::ChooseSaveAs(const std::filesystem::path& directory,
                                    const std::filesystem::path& current,
                                    std::filesystem::path& selected,std::string& error){
#ifdef _WIN32
    return Choose(directory,current,true,selected,error);
#else
    (void)directory;(void)current;selected.clear();error="Native Studio dialogs require Windows";return false;
#endif
}
void StudioFileDialog::ShowError(const std::string& message){
    std::cerr<<"Studio: "<<message<<'\n';
#ifdef _WIN32
    MessageBoxA(GetActiveWindow(),message.c_str(),"Subspace Studio",MB_OK|MB_ICONWARNING);
#endif
}
}
