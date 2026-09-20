#pragma once
// Temporary compatibility bridge for standalone Studio's OpenGL 1.20 overlay.
// The shared render-state abstraction should eventually own this lifetime.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#include <cstdint>
#endif

namespace subspace {
class StudioOverlayProgramScope final {
public:
    StudioOverlayProgramScope() noexcept {
#ifdef _WIN32
        // Avoid querying GL 2.0 state on a missing context or a GL 1.1 driver.
        if (!wglGetCurrentContext()) return;
        PROC raw = wglGetProcAddress("glUseProgram");
        const auto addr = reinterpret_cast<std::uintptr_t>(raw);
        // WGL explicitly allows these sentinel values for missing extensions.
        if (!raw || addr == 1 || addr == 2 || addr == 3 || addr == ~std::uintptr_t{0}) return;
        use_ = reinterpret_cast<UseProgram>(raw);
        // GL_CURRENT_PROGRAM (GL 2.0). Do not require newer GL headers.
        glGetIntegerv(static_cast<GLenum>(0x8B8D), &previous_);
        if (previous_ != 0) use_(0);
#endif
    }
    ~StudioOverlayProgramScope() noexcept {
#ifdef _WIN32
        if (use_ && previous_ != 0) use_(static_cast<GLuint>(previous_));
#endif
    }
    StudioOverlayProgramScope(const StudioOverlayProgramScope&) = delete;
    StudioOverlayProgramScope& operator=(const StudioOverlayProgramScope&) = delete;
private:
#ifdef _WIN32
    using UseProgram = void (APIENTRY *)(GLuint);
    UseProgram use_ = nullptr;
    GLint previous_ = 0;
#endif
};
} // namespace subspace
