#pragma once

#include "core/Math.h"
#include "ships/Ship.h"

#include <string>
#include <vector>
#include <cstdint>

namespace subspace {

/// Color representation for debug drawing (0-255 per channel).
struct DebugColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    static DebugColor Red()    { return {255, 0,   0,   255}; }
    static DebugColor Green()  { return {0,   255, 0,   255}; }
    static DebugColor Blue()   { return {0,   0,   255, 255}; }
    static DebugColor Yellow() { return {255, 255, 0,   255}; }
    static DebugColor Cyan()   { return {0,   255, 255, 255}; }
    static DebugColor White()  { return {255, 255, 255, 255}; }

    bool operator==(const DebugColor& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    bool operator!=(const DebugColor& o) const { return !(*this == o); }
};

/// A single debug draw primitive queued for rendering.
struct DebugDrawCommand {
    enum class Type { Line, Box, Sphere, Text };

    Type type = Type::Line;

    // Line: start/end   Box: center/halfExtents   Sphere: center + radius
    Vector3 p1;          ///< Start / center
    Vector3 p2;          ///< End / half-extents
    float   radius = 0.0f;

    DebugColor color;
    std::string text;    ///< Only used for Type::Text
    float lifetime = 0.0f; ///< Seconds remaining (0 = single frame)
};

/// Categorised debug overlay that can be toggled.
enum class DebugOverlayType {
    BlockRoles,      ///< Color blocks by BlockType (engine=green, weapon=red, etc.)
    DamageState,     ///< Color blocks by HP percentage (green → red)
    Hardpoints,      ///< Show weapon mount positions
    Capabilities,    ///< HUD bars for mobility/firepower/etc.
    Grid,            ///< Editor grid lines
    Physics          ///< Bounding boxes, velocities
};

/// Non-rendering debug draw queue. Collects primitives each frame for
/// the actual renderer to consume. Useful for tooling, editor overlays,
/// and in-game debug HUD without coupling to a specific graphics API.
class DebugRenderer {
public:
    // ------------------------------------------------------------------
    // Draw primitives
    // ------------------------------------------------------------------

    /// Queue a line segment.
    void DrawLine(const Vector3& start, const Vector3& end, const DebugColor& color, float lifetime = 0.0f);

    /// Queue a wireframe axis-aligned box.
    void DrawBox(const Vector3& center, const Vector3& halfExtents, const DebugColor& color, float lifetime = 0.0f);

    /// Queue a wireframe sphere.
    void DrawSphere(const Vector3& center, float radius, const DebugColor& color, float lifetime = 0.0f);

    /// Queue a world-space text label.
    void DrawText(const Vector3& position, const std::string& text, const DebugColor& color, float lifetime = 0.0f);

    // ------------------------------------------------------------------
    // Ship debug helpers
    // ------------------------------------------------------------------

    /// Generate block-role overlay commands for a ship.
    void DrawBlockRoles(const Ship& ship);

    /// Generate damage-state overlay commands (green → red by HP).
    void DrawDamageOverlay(const Ship& ship);

    // ------------------------------------------------------------------
    // Overlay management
    // ------------------------------------------------------------------

    /// Enable / disable a debug overlay category.
    void SetOverlayEnabled(DebugOverlayType type, bool enabled);

    /// Query whether an overlay category is enabled.
    bool IsOverlayEnabled(DebugOverlayType type) const;

    /// Toggle an overlay category.
    void ToggleOverlay(DebugOverlayType type);

    // ------------------------------------------------------------------
    // Frame lifecycle
    // ------------------------------------------------------------------

    /// Tick lifetimes, remove expired commands.
    void Update(float deltaTime);

    /// Get all queued commands (for the real renderer to consume).
    const std::vector<DebugDrawCommand>& GetCommands() const;

    /// Get only commands of a specific type.
    std::vector<DebugDrawCommand> GetCommandsByType(DebugDrawCommand::Type type) const;

    /// Discard all queued commands.
    void Clear();

    /// Get the number of queued commands.
    size_t GetCommandCount() const;

    /// Get the display name for an overlay type.
    static std::string GetOverlayName(DebugOverlayType type);

private:
    std::vector<DebugDrawCommand> _commands;
    uint32_t _overlayFlags = 0; ///< Bitmask of enabled DebugOverlayType values.
};

} // namespace subspace
