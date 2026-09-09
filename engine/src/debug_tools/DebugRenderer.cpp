#include "debug_tools/DebugRenderer.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// Draw primitives
// ---------------------------------------------------------------------------

void DebugRenderer::DrawLine(const Vector3& start, const Vector3& end,
                             const DebugColor& color, float lifetime) {
    DebugDrawCommand cmd;
    cmd.type     = DebugDrawCommand::Type::Line;
    cmd.p1       = start;
    cmd.p2       = end;
    cmd.color    = color;
    cmd.lifetime = lifetime;
    _commands.push_back(cmd);
}

void DebugRenderer::DrawBox(const Vector3& center, const Vector3& halfExtents,
                            const DebugColor& color, float lifetime) {
    DebugDrawCommand cmd;
    cmd.type     = DebugDrawCommand::Type::Box;
    cmd.p1       = center;
    cmd.p2       = halfExtents;
    cmd.color    = color;
    cmd.lifetime = lifetime;
    _commands.push_back(cmd);
}

void DebugRenderer::DrawSphere(const Vector3& center, float radius,
                               const DebugColor& color, float lifetime) {
    DebugDrawCommand cmd;
    cmd.type     = DebugDrawCommand::Type::Sphere;
    cmd.p1       = center;
    cmd.radius   = radius;
    cmd.color    = color;
    cmd.lifetime = lifetime;
    _commands.push_back(cmd);
}

void DebugRenderer::DrawText(const Vector3& position, const std::string& text,
                             const DebugColor& color, float lifetime) {
    DebugDrawCommand cmd;
    cmd.type     = DebugDrawCommand::Type::Text;
    cmd.p1       = position;
    cmd.text     = text;
    cmd.color    = color;
    cmd.lifetime = lifetime;
    _commands.push_back(cmd);
}

// ---------------------------------------------------------------------------
// Ship debug helpers
// ---------------------------------------------------------------------------

void DebugRenderer::DrawBlockRoles(const Ship& ship) {
    for (const auto& block : ship.blocks) {
        if (!block) continue;

        DebugColor color;
        switch (block->type) {
            case BlockType::Engine:      color = DebugColor::Green();  break;
            case BlockType::WeaponMount: color = DebugColor::Red();    break;
            case BlockType::Generator:   color = DebugColor::Yellow(); break;
            case BlockType::Gyro:        color = DebugColor::Cyan();   break;
            case BlockType::Armor:       color = DebugColor::Blue();   break;
            case BlockType::Cargo:       color = DebugColor::White();  break;
            case BlockType::Hull:
            default:                     color = {128, 128, 128, 255}; break;
        }

        Vector3 center(
            static_cast<float>(block->gridPos.x) + block->size.x * 0.5f,
            static_cast<float>(block->gridPos.y) + block->size.y * 0.5f,
            static_cast<float>(block->gridPos.z) + block->size.z * 0.5f
        );
        Vector3 halfExtents(
            block->size.x * 0.5f,
            block->size.y * 0.5f,
            block->size.z * 0.5f
        );
        DrawBox(center, halfExtents, color);
    }
}

void DebugRenderer::DrawDamageOverlay(const Ship& ship) {
    for (const auto& block : ship.blocks) {
        if (!block) continue;

        float hpFraction = (block->maxHP > 0.0f)
            ? (block->currentHP / block->maxHP)
            : 0.0f;

        // Lerp from red (0 HP) to green (full HP)
        uint8_t r = static_cast<uint8_t>((1.0f - hpFraction) * 255.0f);
        uint8_t g = static_cast<uint8_t>(hpFraction * 255.0f);
        DebugColor color = {r, g, 0, 200};

        Vector3 center(
            static_cast<float>(block->gridPos.x) + block->size.x * 0.5f,
            static_cast<float>(block->gridPos.y) + block->size.y * 0.5f,
            static_cast<float>(block->gridPos.z) + block->size.z * 0.5f
        );
        Vector3 halfExtents(
            block->size.x * 0.5f,
            block->size.y * 0.5f,
            block->size.z * 0.5f
        );
        DrawBox(center, halfExtents, color);
    }
}

// ---------------------------------------------------------------------------
// Overlay management
// ---------------------------------------------------------------------------

void DebugRenderer::SetOverlayEnabled(DebugOverlayType type, bool enabled) {
    uint32_t bit = 1u << static_cast<int>(type);
    if (enabled) {
        _overlayFlags |= bit;
    } else {
        _overlayFlags &= ~bit;
    }
}

bool DebugRenderer::IsOverlayEnabled(DebugOverlayType type) const {
    uint32_t bit = 1u << static_cast<int>(type);
    return (_overlayFlags & bit) != 0;
}

void DebugRenderer::ToggleOverlay(DebugOverlayType type) {
    SetOverlayEnabled(type, !IsOverlayEnabled(type));
}

// ---------------------------------------------------------------------------
// Frame lifecycle
// ---------------------------------------------------------------------------

void DebugRenderer::Update(float deltaTime) {
    auto it = _commands.begin();
    while (it != _commands.end()) {
        // Commands with lifetime <= 0 are single-frame — already consumed.
        if (it->lifetime <= 0.0f) {
            it = _commands.erase(it);
        } else {
            it->lifetime -= deltaTime;
            if (it->lifetime <= 0.0f) {
                it = _commands.erase(it);
            } else {
                ++it;
            }
        }
    }
}

const std::vector<DebugDrawCommand>& DebugRenderer::GetCommands() const {
    return _commands;
}

std::vector<DebugDrawCommand> DebugRenderer::GetCommandsByType(
        DebugDrawCommand::Type type) const {
    std::vector<DebugDrawCommand> result;
    for (const auto& cmd : _commands) {
        if (cmd.type == type) result.push_back(cmd);
    }
    return result;
}

void DebugRenderer::Clear() {
    _commands.clear();
}

size_t DebugRenderer::GetCommandCount() const {
    return _commands.size();
}

std::string DebugRenderer::GetOverlayName(DebugOverlayType type) {
    switch (type) {
        case DebugOverlayType::BlockRoles:    return "Block Roles";
        case DebugOverlayType::DamageState:   return "Damage State";
        case DebugOverlayType::Hardpoints:    return "Hardpoints";
        case DebugOverlayType::Capabilities:  return "Capabilities";
        case DebugOverlayType::Grid:          return "Grid";
        case DebugOverlayType::Physics:       return "Physics";
    }
    return "Unknown";
}

} // namespace subspace
