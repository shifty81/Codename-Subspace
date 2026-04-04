#ifdef SUBSPACE_USE_GLFW

#include "rendering/GLFWWindow.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

namespace subspace {

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

GLFWWindow::GLFWWindow() = default;

GLFWWindow::~GLFWWindow()
{
    Destroy();
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool GLFWWindow::Create(int width, int height, const std::string& title)
{
    if (!glfwInit()) {
        std::cerr << "[GLFWWindow] glfwInit() failed.\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    _window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!_window) {
        std::cerr << "[GLFWWindow] Failed to create window.\n";
        glfwTerminate();
        return false;
    }

    _width  = width;
    _height = height;

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // V-sync

    // Set up a 2-D orthographic projection with the origin at top-left,
    // matching UIRenderer screen coordinates.
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(width),
            static_cast<double>(height), 0.0,
            -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void GLFWWindow::Destroy()
{
    if (_window) {
        glfwDestroyWindow(_window);
        _window = nullptr;
        glfwTerminate();
    }
}

// ---------------------------------------------------------------------------
// Frame helpers
// ---------------------------------------------------------------------------

bool GLFWWindow::ShouldClose() const
{
    return _window && glfwWindowShouldClose(_window);
}

void GLFWWindow::PollEvents()
{
    if (!_window) return;

    glfwPollEvents();

    // Handle resize so the viewport stays correct.
    int w, h;
    glfwGetFramebufferSize(_window, &w, &h);
    if (w != _width || h != _height) {
        _width  = w;
        _height = h;
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, static_cast<double>(w),
                static_cast<double>(h), 0.0,
                -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
    }
}

void GLFWWindow::BeginRender()
{
    // Dark-space background.
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLFWWindow::RenderDrawCommands(const UIRenderer& renderer)
{
    for (const auto& cmd : renderer.GetCommands()) {
        switch (cmd.type) {
            case DrawCommandType::FilledRect:   RenderFilledRect(cmd);    break;
            case DrawCommandType::OutlineRect:  RenderOutlineRect(cmd);   break;
            case DrawCommandType::Line:         RenderLine(cmd);          break;
            case DrawCommandType::Text:         RenderText(cmd);          break;
            case DrawCommandType::Circle:       RenderCircle(cmd, false); break;
            case DrawCommandType::FilledCircle: RenderCircle(cmd, true);  break;
        }
    }
}

void GLFWWindow::EndRender()
{
    if (_window) {
        glfwSwapBuffers(_window);
    }
}

// ---------------------------------------------------------------------------
// Individual draw-command renderers (legacy GL immediate mode)
// ---------------------------------------------------------------------------

void GLFWWindow::RenderFilledRect(const DrawCommand& cmd)
{
    glColor4f(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
    glBegin(GL_QUADS);
    glVertex2f(cmd.rect.x,                  cmd.rect.y);
    glVertex2f(cmd.rect.x + cmd.rect.width, cmd.rect.y);
    glVertex2f(cmd.rect.x + cmd.rect.width, cmd.rect.y + cmd.rect.height);
    glVertex2f(cmd.rect.x,                  cmd.rect.y + cmd.rect.height);
    glEnd();
}

void GLFWWindow::RenderOutlineRect(const DrawCommand& cmd)
{
    glColor4f(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
    glLineWidth(cmd.lineWidth);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cmd.rect.x,                  cmd.rect.y);
    glVertex2f(cmd.rect.x + cmd.rect.width, cmd.rect.y);
    glVertex2f(cmd.rect.x + cmd.rect.width, cmd.rect.y + cmd.rect.height);
    glVertex2f(cmd.rect.x,                  cmd.rect.y + cmd.rect.height);
    glEnd();
}

void GLFWWindow::RenderLine(const DrawCommand& cmd)
{
    glColor4f(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
    glLineWidth(cmd.lineWidth);
    glBegin(GL_LINES);
    glVertex2f(cmd.p1.x, cmd.p1.y);
    glVertex2f(cmd.p2.x, cmd.p2.y);
    glEnd();
}

void GLFWWindow::RenderText(const DrawCommand& cmd)
{
    // Minimal text placeholder — render each character as a small filled
    // rectangle.  A real implementation would use FreeType or a bitmap-font
    // atlas, but this gets visible output on-screen immediately.
    if (cmd.text.empty()) return;

    const float charW = cmd.fontSize * 0.55f;
    const float charH = static_cast<float>(cmd.fontSize);
    const float gap   = cmd.fontSize * 0.1f;

    glColor4f(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

    float x = cmd.p1.x;
    float y = cmd.p1.y;
    for (size_t i = 0; i < cmd.text.size(); ++i) {
        if (cmd.text[i] == ' ') {
            x += charW + gap;
            continue;
        }
        glBegin(GL_QUADS);
        glVertex2f(x,          y);
        glVertex2f(x + charW,  y);
        glVertex2f(x + charW,  y + charH);
        glVertex2f(x,          y + charH);
        glEnd();
        x += charW + gap;
    }
}

void GLFWWindow::RenderCircle(const DrawCommand& cmd, bool filled)
{
    const float cx     = cmd.p1.x;
    const float cy     = cmd.p1.y;
    const float radius = cmd.p2.x;
    constexpr int kSegments = 48;
    constexpr float kTwoPi  = 2.0f * 3.14159265358979f;

    glColor4f(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

    if (filled) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
    } else {
        glLineWidth(cmd.lineWidth);
        glBegin(GL_LINE_LOOP);
    }

    for (int i = 0; i <= kSegments; ++i) {
        float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(kSegments);
        glVertex2f(cx + radius * std::cos(angle),
                   cy + radius * std::sin(angle));
    }

    glEnd();
}

} // namespace subspace

#endif // SUBSPACE_USE_GLFW
