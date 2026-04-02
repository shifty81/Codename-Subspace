#pragma once

#include <array>
#include <cstdint>

namespace subspace {
namespace Input {

/// Platform-independent key codes.
enum class KeyCode : uint32_t {
    // Letter keys
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Digit row
    Key0, Key1, Key2, Key3, Key4,
    Key5, Key6, Key7, Key8, Key9,

    // Special / modifier keys
    Space, Enter, Escape, Backspace, Tab,
    LeftShift, RightShift,
    LeftCtrl,  RightCtrl,
    LeftAlt,   RightAlt,

    // Navigation
    ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
    Home, End, PageUp, PageDown,
    Insert, Delete,

    // Function keys
    F1, F2,  F3,  F4,  F5,  F6,
    F7, F8,  F9, F10, F11, F12,

    Count ///< Sentinel — keep last.
};

/// Tracks per-frame keyboard state.
///
/// Call Update() once per frame with the new set of held keys.
class Keyboard {
public:
    /// Returns true while a key is held down.
    bool IsKeyDown(KeyCode key) const noexcept;

    /// Returns true only on the first frame the key is pressed.
    bool IsKeyPressed(KeyCode key) const noexcept;

    /// Returns true only on the frame the key is released.
    bool IsKeyReleased(KeyCode key) const noexcept;

    /// Advance to the next frame.
    /// @param newState  Array indexed by KeyCode where true = key is held.
    void Update(const std::array<bool,
                static_cast<std::size_t>(KeyCode::Count)>& newState) noexcept;

    /// Set a single key state directly (convenience for unit tests).
    void SetKey(KeyCode key, bool down) noexcept;

private:
    using KeyArray = std::array<bool, static_cast<std::size_t>(KeyCode::Count)>;
    KeyArray _current{};
    KeyArray _previous{};
};

} // namespace Input
} // namespace subspace
