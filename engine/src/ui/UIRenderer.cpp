#include "ui/UIRenderer.h"

namespace subspace {

void UIRenderer::BeginFrame(float screenWidth, float screenHeight) {
    _commands.clear();
    _screenWidth  = screenWidth;
    _screenHeight = screenHeight;
}

void UIRenderer::EndFrame() {
    // Reserved for command sorting / batching optimizations.
}

void UIRenderer::Submit(const std::vector<DrawCommand>& commands) {
    _commands.insert(_commands.end(), commands.begin(), commands.end());
}

void UIRenderer::DrawFilledRect(const Rect& rect, const Color& color) {
    DrawCommand cmd;
    cmd.type  = DrawCommandType::FilledRect;
    cmd.rect  = rect;
    cmd.color = color;
    _commands.push_back(cmd);
}

void UIRenderer::DrawOutlineRect(const Rect& rect, const Color& color, float lineWidth) {
    DrawCommand cmd;
    cmd.type      = DrawCommandType::OutlineRect;
    cmd.rect      = rect;
    cmd.color     = color;
    cmd.lineWidth = lineWidth;
    _commands.push_back(cmd);
}

void UIRenderer::DrawText(const std::string& text, const Vec2& pos,
                           const Color& color, int fontSize) {
    DrawCommand cmd;
    cmd.type     = DrawCommandType::Text;
    cmd.text     = text;
    cmd.p1       = pos;
    cmd.color    = color;
    cmd.fontSize = fontSize;
    _commands.push_back(cmd);
}

void UIRenderer::DrawLine(const Vec2& from, const Vec2& to,
                           const Color& color, float lineWidth) {
    DrawCommand cmd;
    cmd.type      = DrawCommandType::Line;
    cmd.p1        = from;
    cmd.p2        = to;
    cmd.color     = color;
    cmd.lineWidth = lineWidth;
    _commands.push_back(cmd);
}

void UIRenderer::DrawCircle(const Vec2& center, float radius,
                             const Color& color, float lineWidth) {
    DrawCommand cmd;
    cmd.type      = DrawCommandType::Circle;
    cmd.p1        = center;
    cmd.p2        = {radius, 0.0f};
    cmd.color     = color;
    cmd.lineWidth = lineWidth;
    _commands.push_back(cmd);
}

void UIRenderer::DrawFilledCircle(const Vec2& center, float radius,
                                   const Color& color) {
    DrawCommand cmd;
    cmd.type  = DrawCommandType::FilledCircle;
    cmd.p1    = center;
    cmd.p2    = {radius, 0.0f};
    cmd.color = color;
    _commands.push_back(cmd);
}

} // namespace subspace
