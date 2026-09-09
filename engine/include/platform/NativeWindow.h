#pragma once

#include "input/InputState.h"

#include <string>

namespace subspace {

struct NativeWindowConfig {
    std::string title = "Codename: Subspace";
    int width = 1600;
    int height = 900;
    bool resizable = true;
    bool vsync = true;
};

/// Native platform window + OpenGL presentation authority.
/// On Windows this owns HWND/HDC/HGLRC directly through Win32/WGL, replacing
/// the old Silk.NET Windowing/OpenGL ownership. Non-Windows builds expose a
/// compile-safe stub until another platform backend is added.
class NativeWindow {
public:
    explicit NativeWindow(InputState& inputState);
    ~NativeWindow();

    NativeWindow(const NativeWindow&) = delete;
    NativeWindow& operator=(const NativeWindow&) = delete;

    bool Initialize(const NativeWindowConfig& config = {});
    void Shutdown();

    /// Pump OS messages. Returns false once the window is closing.
    bool PumpEvents();

    /// Clear the current OpenGL backbuffer for a new frame.
    void BeginFrame(float r = 0.015f, float g = 0.020f, float b = 0.035f, float a = 1.0f);

    /// Present the current OpenGL backbuffer.
    void EndFrame();

    bool IsOpen() const { return _open; }
    int GetWidth() const { return _width; }
    int GetHeight() const { return _height; }
    const NativeWindowConfig& GetConfig() const { return _config; }

    /// Consume a pending primary-button click in client coordinates.
    bool ConsumePrimaryClick(float& x, float& y);

    /// Primary-button lifecycle for editor-grade drag operations. A drag no
    /// longer also generates a click when the pointer moved beyond the click
    /// threshold. Press/release coordinates remain in client pixels.
    bool ConsumePrimaryPress(float& x, float& y);
    bool ConsumePrimaryDragDelta(float& deltaX, float& deltaY);
    bool ConsumePrimaryRelease(float& x, float& y);
    bool IsPrimaryButtonDown() const { return _primaryButtonDown; }
    bool IsControlDown() const { return _controlDown; }
    bool IsShiftDown() const { return _shiftDown; }
    bool IsAltDown() const { return _altDown; }

    /// Consume a pending secondary-button click in client coordinates. A drag
    /// remains camera-orbit input; only a short RMB press/release becomes context.
    bool ConsumeSecondaryClick(float& x, float& y);

    /// Consume accumulated mouse-wheel notches since the previous query.
    float ConsumeWheelDelta();

    /// Consume right-mouse drag deltas used for 360-degree camera orbit.
    bool ConsumeCameraOrbitDelta(float& deltaX, float& deltaY);

    /// Consume middle-mouse drag deltas used for inspection-mode panning.
    bool ConsumeCameraPanDelta(float& deltaX, float& deltaY);

    float GetPointerX() const { return _pointerX; }
    float GetPointerY() const { return _pointerY; }

    /// True only when this build has a real native platform backend.
    static bool IsPlatformBackendAvailable();

private:
#ifdef _WIN32
    static long long __stdcall StaticWindowProc(void* hwnd, unsigned int message,
                                                 unsigned long long wParam,
                                                 long long lParam);
    long long WindowProc(void* hwnd, unsigned int message,
                         unsigned long long wParam, long long lParam);
    void ApplyKey(unsigned long long virtualKey, bool down);
    bool CreateOpenGLContext();
    void DestroyOpenGLContext();

    void* _instance = nullptr;
    void* _window = nullptr;
    void* _deviceContext = nullptr;
    void* _glContext = nullptr;
    unsigned short _classAtom = 0;
#endif

    InputState& _inputState;
    NativeWindowConfig _config{};
    int _width = 0;
    int _height = 0;
    bool _open = false;
    float _pointerX = 0.0f;
    float _pointerY = 0.0f;
    float _pendingClickX = 0.0f;
    float _pendingClickY = 0.0f;
    float _wheelDelta = 0.0f;
    bool _primaryClickPending = false;
    bool _primaryPressPending = false;
    bool _primaryReleasePending = false;
    bool _primaryButtonDown = false;
    float _primaryDownX = 0.0f;
    float _primaryDownY = 0.0f;
    float _lastPrimaryX = 0.0f;
    float _lastPrimaryY = 0.0f;
    float _primaryDragDistance = 0.0f;
    float _primaryDragDeltaX = 0.0f;
    float _primaryDragDeltaY = 0.0f;
    float _pendingPrimaryPressX = 0.0f;
    float _pendingPrimaryPressY = 0.0f;
    float _pendingPrimaryReleaseX = 0.0f;
    float _pendingPrimaryReleaseY = 0.0f;
    bool _controlDown = false;
    bool _shiftDown = false;
    bool _altDown = false;
    bool _secondaryClickPending = false;
    float _pendingSecondaryX = 0.0f;
    float _pendingSecondaryY = 0.0f;
    float _rightDownX = 0.0f;
    float _rightDownY = 0.0f;
    float _rightDragDistance = 0.0f;
    bool _cameraOrbitDragging = false;
    bool _cameraPanDragging = false;
    float _lastOrbitX = 0.0f;
    float _lastOrbitY = 0.0f;
    float _cameraOrbitDeltaX = 0.0f;
    float _cameraOrbitDeltaY = 0.0f;
    float _lastPanX = 0.0f;
    float _lastPanY = 0.0f;
    float _cameraPanDeltaX = 0.0f;
    float _cameraPanDeltaY = 0.0f;
};

} // namespace subspace
