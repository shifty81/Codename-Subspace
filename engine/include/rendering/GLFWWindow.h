#pragma once

#ifdef SUBSPACE_USE_GLFW

#include "ui/UIRenderer.h"

#include <string>

struct GLFWwindow;

namespace subspace {

/// Platform window backed by GLFW + legacy OpenGL.
///
/// When the engine is compiled with `-DSUBSPACE_USE_GLFW=ON`, this class
/// creates a visible window and translates the data-driven DrawCommand
/// buffer produced by UIRenderer into actual OpenGL draw calls each frame.
///
/// Construction is cheap; the actual OS window is created by Create().
class GLFWWindow {
public:
    GLFWWindow();
    ~GLFWWindow();

    // Non-copyable.
    GLFWWindow(const GLFWWindow&) = delete;
    GLFWWindow& operator=(const GLFWWindow&) = delete;

    /// Create the OS window and initialise the OpenGL context.
    /// Returns false on failure (e.g. no display available).
    bool Create(int width, int height, const std::string& title);

    /// Destroy the window and terminate GLFW.
    void Destroy();

    /// True after the user clicks the close button.
    bool ShouldClose() const;

    /// Pump the OS event queue (keyboard, mouse, resize, close, …).
    void PollEvents();

    /// Clear the framebuffer for a new frame.
    void BeginRender();

    /// Walk the UIRenderer command buffer and issue OpenGL calls.
    void RenderDrawCommands(const UIRenderer& renderer);

    /// Swap front/back buffers to present the frame.
    void EndRender();

    int GetWidth()  const { return _width; }
    int GetHeight() const { return _height; }

private:
    void RenderFilledRect(const DrawCommand& cmd);
    void RenderOutlineRect(const DrawCommand& cmd);
    void RenderLine(const DrawCommand& cmd);
    void RenderText(const DrawCommand& cmd);
    void RenderCircle(const DrawCommand& cmd, bool filled);

    GLFWwindow* _window = nullptr;
    int _width  = 1280;
    int _height = 720;
};

} // namespace subspace

#endif // SUBSPACE_USE_GLFW
