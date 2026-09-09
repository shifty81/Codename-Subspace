#include "platform/NativeWindow.h"

#include <algorithm>
#include <cmath>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#endif

namespace subspace {

NativeWindow::NativeWindow(InputState& inputState)
    : _inputState(inputState)
{
}

NativeWindow::~NativeWindow()
{
    Shutdown();
}

bool NativeWindow::IsPlatformBackendAvailable()
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool NativeWindow::Initialize(const NativeWindowConfig& config)
{
    Shutdown();
    _config = config;
    _width = std::max(320, config.width);
    _height = std::max(240, config.height);

#ifdef _WIN32
    // R5 GUI contract: make client pixels, OpenGL viewport pixels and mouse
    // hit-test pixels identical under Windows display scaling. This is done
    // before creating the HWND so main-menu and every workspace control can
    // use the same rectangles without DPI compensation hacks.
    static bool dpiInitialized=false;
    if(!dpiInitialized){ SetProcessDPIAware(); dpiInitialized=true; }
    _instance = GetModuleHandleW(nullptr);

    const wchar_t* className = L"SubspaceNativeOpenGLWindow";
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = reinterpret_cast<WNDPROC>(&NativeWindow::StaticWindowProc);
    wc.hInstance = static_cast<HINSTANCE>(_instance);
    // Use the explicit wide resource form. CMake-generated Visual Studio projects
    // do not automatically define UNICODE, so generic IDC_ARROW can otherwise
    // resolve to the ANSI LPSTR resource type while calling LoadCursorW.
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.lpszClassName = className;

    _classAtom = RegisterClassExW(&wc);
    if (_classAtom == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    if (config.resizable) style |= WS_THICKFRAME | WS_MAXIMIZEBOX;

    RECT rect{0, 0, _width, _height};
    AdjustWindowRect(&rect, style, FALSE);

    std::wstring title(config.title.begin(), config.title.end());
    HWND hwnd = CreateWindowExW(
        0, className, title.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, static_cast<HINSTANCE>(_instance), this);
    if (!hwnd) return false;

    _window = hwnd;
    _deviceContext = GetDC(hwnd);
    if (!_deviceContext || !CreateOpenGLContext()) {
        Shutdown();
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    glViewport(0, 0, _width, _height);
    _open = true;
    return true;
#else
    (void)config;
    _open = false;
    return false;
#endif
}

void NativeWindow::Shutdown()
{
#ifdef _WIN32
    DestroyOpenGLContext();

    if (_window) {
        HWND hwnd = static_cast<HWND>(_window);
        if (_deviceContext) {
            ReleaseDC(hwnd, static_cast<HDC>(_deviceContext));
        }
        DestroyWindow(hwnd);
    }

    _deviceContext = nullptr;
    _window = nullptr;

    if (_classAtom != 0 && _instance) {
        UnregisterClassW(L"SubspaceNativeOpenGLWindow", static_cast<HINSTANCE>(_instance));
    }
    _classAtom = 0;
    _instance = nullptr;
#endif
    _inputState.Clear();
    _open = false;
}

bool NativeWindow::ConsumePrimaryClick(float& x, float& y)
{
    if (!_primaryClickPending) return false;
    x = _pendingClickX;
    y = _pendingClickY;
    _primaryClickPending = false;
    return true;
}

bool NativeWindow::ConsumePrimaryPress(float& x, float& y)
{
    if (!_primaryPressPending) return false;
    x = _pendingPrimaryPressX;
    y = _pendingPrimaryPressY;
    _primaryPressPending = false;
    return true;
}

bool NativeWindow::ConsumePrimaryDragDelta(float& deltaX, float& deltaY)
{
    deltaX = _primaryDragDeltaX;
    deltaY = _primaryDragDeltaY;
    _primaryDragDeltaX = 0.0f;
    _primaryDragDeltaY = 0.0f;
    return std::abs(deltaX) > 0.001f || std::abs(deltaY) > 0.001f;
}

bool NativeWindow::ConsumePrimaryRelease(float& x, float& y)
{
    if (!_primaryReleasePending) return false;
    x = _pendingPrimaryReleaseX;
    y = _pendingPrimaryReleaseY;
    _primaryReleasePending = false;
    return true;
}

bool NativeWindow::ConsumeSecondaryClick(float& x, float& y)
{
    if (!_secondaryClickPending) return false;
    x = _pendingSecondaryX;
    y = _pendingSecondaryY;
    _secondaryClickPending = false;
    return true;
}

float NativeWindow::ConsumeWheelDelta()
{
    const float delta = _wheelDelta;
    _wheelDelta = 0.0f;
    return delta;
}

bool NativeWindow::ConsumeCameraOrbitDelta(float& deltaX, float& deltaY)
{
    deltaX = _cameraOrbitDeltaX;
    deltaY = _cameraOrbitDeltaY;
    _cameraOrbitDeltaX = 0.0f;
    _cameraOrbitDeltaY = 0.0f;
    return std::abs(deltaX) > 0.001f || std::abs(deltaY) > 0.001f;
}

bool NativeWindow::ConsumeCameraPanDelta(float& deltaX, float& deltaY)
{
    deltaX = _cameraPanDeltaX;
    deltaY = _cameraPanDeltaY;
    _cameraPanDeltaX = 0.0f;
    _cameraPanDeltaY = 0.0f;
    return std::abs(deltaX) > 0.001f || std::abs(deltaY) > 0.001f;
}

bool NativeWindow::PumpEvents()
{
#ifdef _WIN32
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            _open = false;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
#endif
    return _open;
}

void NativeWindow::BeginFrame(float r, float g, float b, float a)
{
#ifdef _WIN32
    if (!_open || !_glContext) return;
    glViewport(0, 0, _width, _height);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    (void)r; (void)g; (void)b; (void)a;
#endif
}

void NativeWindow::EndFrame()
{
#ifdef _WIN32
    if (_open && _deviceContext) {
        SwapBuffers(static_cast<HDC>(_deviceContext));
    }
#endif
}

#ifdef _WIN32

bool NativeWindow::CreateOpenGLContext()
{
    HDC hdc = static_cast<HDC>(_deviceContext);

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0 || !SetPixelFormat(hdc, pixelFormat, &pfd)) {
        return false;
    }

    HGLRC context = wglCreateContext(hdc);
    if (!context || !wglMakeCurrent(hdc, context)) {
        if (context) wglDeleteContext(context);
        return false;
    }

    _glContext = context;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    return true;
}

void NativeWindow::DestroyOpenGLContext()
{
    if (_glContext) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(static_cast<HGLRC>(_glContext));
        _glContext = nullptr;
    }
}

void NativeWindow::ApplyKey(unsigned long long virtualKey, bool down)
{
    switch (static_cast<WPARAM>(virtualKey)) {
        case 'W': _inputState.SetAction(InputAction::ThrustForward, down); _inputState.SetAction(InputAction::EditorToolMove, down); break;
        case 'S': _inputState.SetAction(InputAction::ThrustReverse, down); break;
        case 'A': _inputState.SetAction(InputAction::StrafeLeft, down); break;
        case 'D': _inputState.SetAction(InputAction::StrafeRight, down); break;
        case 'Q': _inputState.SetAction(InputAction::TurnLeft, down); _inputState.SetAction(InputAction::EditorToolSelect, down); break;
        case 'E': _inputState.SetAction(InputAction::TurnRight, down); _inputState.SetAction(InputAction::EditorToolRotate, down); break;
        case 'X': _inputState.SetAction(InputAction::EmergencyBrake, down); break;
        case 'V': _inputState.SetAction(InputAction::ToggleDampening, down); break;
        case VK_SPACE: _inputState.SetAction(InputAction::FirePrimary, down); break;
        case 'F': _inputState.SetAction(InputAction::FireMiningMissile, down); _inputState.SetAction(InputAction::EditorFrameSelected, down); break;
        case VK_HOME: _inputState.SetAction(InputAction::EditorFrameShip, down); break;
        case 'J': _inputState.SetAction(InputAction::RequestDock, down); break;
        case 'I': _inputState.SetAction(InputAction::ToggleInterior, down); break;
        case 'M': _inputState.SetAction(InputAction::OpenGalaxyMap, down); break;
        case 'N': _inputState.SetAction(InputAction::OpenSystemMap, down); break;
        case 'R': _inputState.SetAction(InputAction::OpenPlanetSurvey, down); _inputState.SetAction(InputAction::EditorToolScale, down); break;
        case 'P': _inputState.SetAction(InputAction::OpenPlanetaryManufacturing, down); break;
        case 'B': _inputState.SetAction(InputAction::OpenStationBuilder, down); break;
        case 'K': _inputState.SetAction(InputAction::OpenShipBuilder, down); break;
        case 'H': _inputState.SetAction(InputAction::OpenHangarFitting, down); break;
        case 'T': _inputState.SetAction(InputAction::OpenMarketContracts, down); break;
        case 'O': _inputState.SetAction(InputAction::OpenExploration, down); break;
        case 'G': _inputState.SetAction(InputAction::OpenFleetCorporation, down); break;
        case VK_TAB: _inputState.SetAction(InputAction::ToggleFlightMode, down); break;
        case VK_SHIFT: _shiftDown=down; _inputState.SetAction(InputAction::Boost, down); break;
        case VK_CONTROL: _controlDown=down; break;
        case VK_MENU: _altDown=down; break;
        case VK_F6: _inputState.SetAction(InputAction::ToggleShipInspection, down); break;
        case VK_RETURN: _inputState.SetAction(InputAction::MenuAccept, down); break;
        case VK_DOWN: _inputState.SetAction(InputAction::MenuNext, down); _inputState.SetAction(InputAction::EditorNudgeAft, down); break;
        case VK_UP: _inputState.SetAction(InputAction::MenuPrevious, down); _inputState.SetAction(InputAction::EditorNudgeForward, down); break;
        case VK_LEFT: _inputState.SetAction(InputAction::EditorNudgeLeft, down); break;
        case VK_RIGHT: _inputState.SetAction(InputAction::EditorNudgeRight, down); break;
        case VK_PRIOR: _inputState.SetAction(InputAction::EditorNudgeUp, down); break;
        case VK_NEXT: _inputState.SetAction(InputAction::EditorNudgeDown, down); break;
        case VK_DELETE: _inputState.SetAction(InputAction::EditorDeleteModule, down); break;
        case VK_BACK: _inputState.SetAction(InputAction::MenuBack, down); break;
        case VK_ESCAPE:
            _inputState.SetAction(InputAction::Pause, down);
            _inputState.SetAction(InputAction::MenuBack, down);
            break;
        default: break;
    }
}

long long __stdcall NativeWindow::StaticWindowProc(void* hwndRaw, unsigned int message,
                                                    unsigned long long wParam,
                                                    long long lParam)
{
    HWND hwnd = static_cast<HWND>(hwndRaw);
    NativeWindow* self = reinterpret_cast<NativeWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<NativeWindow*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }

    if (self) {
        return self->WindowProc(hwndRaw, message, wParam, lParam);
    }
    return static_cast<long long>(DefWindowProcW(hwnd, message, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)));
}

long long NativeWindow::WindowProc(void* hwndRaw, unsigned int message,
                                   unsigned long long wParam, long long lParam)
{
    HWND hwnd = static_cast<HWND>(hwndRaw);
    switch (message) {
        case WM_CLOSE:
            _open = false;
            PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            _open = false;
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            _width = std::max(1, static_cast<int>(LOWORD(static_cast<LPARAM>(lParam))));
            _height = std::max(1, static_cast<int>(HIWORD(static_cast<LPARAM>(lParam))));
            if (_glContext) glViewport(0, 0, _width, _height);
            return 0;

        case WM_KILLFOCUS:
            _inputState.Clear();
            _primaryButtonDown=false; _primaryPressPending=false; _primaryReleasePending=false; _primaryDragDeltaX=0.0f; _primaryDragDeltaY=0.0f; _cameraOrbitDragging=false; _cameraPanDragging=false; _altDown=false;
            return 0;

        case WM_MOUSEMOVE: {
            _pointerX = static_cast<float>(static_cast<short>(LOWORD(static_cast<LPARAM>(lParam))));
            _pointerY = static_cast<float>(static_cast<short>(HIWORD(static_cast<LPARAM>(lParam))));
            if (_primaryButtonDown) {
                const float dx = _pointerX - _lastPrimaryX;
                const float dy = _pointerY - _lastPrimaryY;
                _primaryDragDeltaX += dx;
                _primaryDragDeltaY += dy;
                _primaryDragDistance += std::sqrt(dx*dx + dy*dy);
                _lastPrimaryX = _pointerX;
                _lastPrimaryY = _pointerY;
            }
            if (_cameraOrbitDragging) {
                const float dx = _pointerX - _lastOrbitX;
                const float dy = _pointerY - _lastOrbitY;
                _cameraOrbitDeltaX += dx;
                _cameraOrbitDeltaY += dy;
                _rightDragDistance += std::sqrt(dx*dx + dy*dy);
                _lastOrbitX = _pointerX;
                _lastOrbitY = _pointerY;
            }
            if (_cameraPanDragging) {
                _cameraPanDeltaX += _pointerX - _lastPanX;
                _cameraPanDeltaY += _pointerY - _lastPanY;
                _lastPanX = _pointerX;
                _lastPanY = _pointerY;
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
            _pointerX = static_cast<float>(static_cast<short>(LOWORD(static_cast<LPARAM>(lParam))));
            _pointerY = static_cast<float>(static_cast<short>(HIWORD(static_cast<LPARAM>(lParam))));
            _primaryButtonDown = true;
            _primaryDownX = _lastPrimaryX = _pointerX;
            _primaryDownY = _lastPrimaryY = _pointerY;
            _primaryDragDistance = 0.0f;
            _primaryDragDeltaX = _primaryDragDeltaY = 0.0f;
            _pendingPrimaryPressX = _pointerX; _pendingPrimaryPressY = _pointerY;
            _primaryPressPending = true;
            SetCapture(hwnd);
            return 0;

        case WM_LBUTTONUP:
            _pointerX = static_cast<float>(static_cast<short>(LOWORD(static_cast<LPARAM>(lParam))));
            _pointerY = static_cast<float>(static_cast<short>(HIWORD(static_cast<LPARAM>(lParam))));
            if(_primaryButtonDown){
                const float dx = _pointerX - _lastPrimaryX;
                const float dy = _pointerY - _lastPrimaryY;
                _primaryDragDeltaX += dx; _primaryDragDeltaY += dy;
                _primaryDragDistance += std::sqrt(dx*dx + dy*dy);
                _pendingPrimaryReleaseX = _pointerX; _pendingPrimaryReleaseY = _pointerY;
                _primaryReleasePending = true;
                if(_primaryDragDistance < 5.0f){
                    _pendingClickX = _pointerX; _pendingClickY = _pointerY;
                    _primaryClickPending = true;
                }
            }
            _primaryButtonDown = false;
            ReleaseCapture();
            return 0;

        case WM_RBUTTONDOWN:
            _pointerX = static_cast<float>(static_cast<short>(LOWORD(static_cast<LPARAM>(lParam))));
            _pointerY = static_cast<float>(static_cast<short>(HIWORD(static_cast<LPARAM>(lParam))));
            _cameraOrbitDragging = true;
            _lastOrbitX = _pointerX;
            _lastOrbitY = _pointerY;
            _rightDownX = _pointerX;
            _rightDownY = _pointerY;
            _rightDragDistance = 0.0f;
            SetCapture(hwnd);
            return 0;

        case WM_RBUTTONUP:
            _pointerX = static_cast<float>(static_cast<short>(LOWORD(static_cast<LPARAM>(lParam))));
            _pointerY = static_cast<float>(static_cast<short>(HIWORD(static_cast<LPARAM>(lParam))));
            _cameraOrbitDragging = false;
            if (_rightDragDistance < 5.0f) {
                _pendingSecondaryX = _pointerX;
                _pendingSecondaryY = _pointerY;
                _secondaryClickPending = true;
            }
            ReleaseCapture();
            return 0;

        case WM_MBUTTONDOWN:
            _cameraPanDragging = true;
            _lastPanX = _pointerX;
            _lastPanY = _pointerY;
            SetCapture(hwnd);
            return 0;

        case WM_MBUTTONUP:
            _cameraPanDragging = false;
            ReleaseCapture();
            return 0;

        case WM_MOUSEWHEEL:
            _wheelDelta += static_cast<float>(GET_WHEEL_DELTA_WPARAM(static_cast<WPARAM>(wParam))) / static_cast<float>(WHEEL_DELTA);
            return 0;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            ApplyKey(wParam, true);
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            ApplyKey(wParam, false);
            return 0;

        default:
            break;
    }

    return static_cast<long long>(DefWindowProcW(hwnd, message, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)));
}

#endif // _WIN32

} // namespace subspace
