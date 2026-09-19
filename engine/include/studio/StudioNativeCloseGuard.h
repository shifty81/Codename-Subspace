#pragma once
#include <functional>
#include <string>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#endif

namespace subspace {
// Installed ONLY by Studio after NativeWindow initializes. Intercepts WM_CLOSE
// before the shared game/native window handler posts WM_QUIT. Never changes
// NativeGameApplication, platform window source, or gameplay's close behavior.
class StudioNativeCloseGuard {
public:
    StudioNativeCloseGuard() = default;
    StudioNativeCloseGuard(const StudioNativeCloseGuard&) = delete;
    StudioNativeCloseGuard& operator=(const StudioNativeCloseGuard&) = delete;
    ~StudioNativeCloseGuard(){ Detach(); }
    bool Install(const std::string& expectedTitle, std::function<bool()> mayClose,
                 std::string& error);
    void Detach() noexcept;
private:
    void* window_ = nullptr;
    std::function<bool()> mayClose_;
#ifdef _WIN32
    static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam,
                                         LPARAM lParam, UINT_PTR id, DWORD_PTR refData);
#endif
};
}
