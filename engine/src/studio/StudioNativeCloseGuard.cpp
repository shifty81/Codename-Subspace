#include "studio/StudioNativeCloseGuard.h"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include <cwchar>
#endif

namespace subspace {
bool StudioNativeCloseGuard::Install(const std::string& expectedTitle,
                                    std::function<bool()> mayClose,
                                    std::string& error){
    error.clear();
    if(!mayClose){error="Studio close approval callback is missing";return false;}
#ifdef _WIN32
    if(window_){error="Studio close guard is already attached";return false;}
    // Only attach to THIS thread's exact Studio HWND, never to an unrelated
    // window or a dialog that happened to have foreground focus.
    const auto title=std::wstring(expectedTitle.begin(),expectedTitle.end());
    struct Search { const std::wstring* title; HWND found; } search{&title,nullptr};
    EnumThreadWindows(GetCurrentThreadId(),[](HWND hwnd,LPARAM context)->BOOL{
        auto& find=*reinterpret_cast<Search*>(context);
        wchar_t buffer[512]{};
        const int size=GetWindowTextW(hwnd,buffer,512);
        if(size>0 && std::wstring(buffer,static_cast<std::size_t>(size))==*find.title){
            find.found=hwnd;return FALSE;
        }
        return TRUE;
    },reinterpret_cast<LPARAM>(&search));
    if(!search.found){error="Studio top-level window was not found on its GUI thread";return false;}
    mayClose_=std::move(mayClose);
    const auto id=reinterpret_cast<UINT_PTR>(this);
    if(!SetWindowSubclass(search.found,
       &StudioNativeCloseGuard::SubclassProc,id,
       reinterpret_cast<DWORD_PTR>(this))){
        mayClose_={};error="Cannot install Studio-only close interception";return false;
    }
    window_=search.found;
#else
    (void)expectedTitle;
    error="Studio native close interception requires a Windows window";
    return false;
#endif
    return true;
}
void StudioNativeCloseGuard::Detach() noexcept {
#ifdef _WIN32
    if(window_){
        RemoveWindowSubclass(static_cast<HWND>(window_),
            &StudioNativeCloseGuard::SubclassProc,
            reinterpret_cast<UINT_PTR>(this));
    }
#endif
    window_=nullptr;mayClose_={};
}
#ifdef _WIN32
LRESULT CALLBACK StudioNativeCloseGuard::SubclassProc(HWND hwnd,UINT message,
                         WPARAM wParam,LPARAM lParam,
                         UINT_PTR id,DWORD_PTR reference){
    auto* self=reinterpret_cast<StudioNativeCloseGuard*>(reference);
    if(message==WM_CLOSE && self && self->mayClose_ && !self->mayClose_())return 0;
    if(message==WM_NCDESTROY && self){
        self->window_=nullptr;
        RemoveWindowSubclass(hwnd,
            &StudioNativeCloseGuard::SubclassProc,
            static_cast<UINT_PTR>(id));
    }
    return DefSubclassProc(hwnd,message,wParam,lParam);
}
#endif
}
