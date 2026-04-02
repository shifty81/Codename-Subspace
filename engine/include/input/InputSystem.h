#pragma once

#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "input/Gamepad.h"
#include "core/math/Vector2.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/persistence/SaveGameManager.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {
namespace Input {

/// What value an InputAction produces.
enum class ActionType {
    Button, ///< Boolean pressed / released.
    Axis1D, ///< Single float value.
    Axis2D  ///< Two-dimensional vector.
};

/// A named logical input action bound to one or more keys.
///
/// Multiple keys may be bound; any held key satisfies a Button action.
struct InputAction {
    std::string        name;
    ActionType         type = ActionType::Button;
    std::vector<KeyCode> bindings;

    /// Query the boolean state from keyboard.
    bool GetBool(const Keyboard& kb) const;

    /// Query as a scalar (1 if any binding held, else 0).
    float GetFloat(const Keyboard& kb) const;

    /// Query as a 2-D vector.  First binding → (1,0), second → (-1,0),
    /// third → (0,1), fourth → (0,-1); additional bindings ignored.
    Vector2 GetVector2(const Keyboard& kb) const;
};

/// Manages named actions and their bindings.  Updated once per frame.
class InputSystem : public SystemBase {
public:
    InputSystem();

    void Update(float /*deltaTime*/) override {}

    /// Register an action.  Replaces any existing action with the same name.
    void RegisterAction(InputAction action);

    /// Bind an additional key to a named action.
    void BindKey(const std::string& actionName, KeyCode key);

    /// Advance device state for the current frame.
    /// Call this once per frame before querying actions.
    void PollDevices(
        const std::array<bool, static_cast<std::size_t>(KeyCode::Count)>& keys,
        float mouseX, float mouseY, uint8_t mouseButtons,
        float mouseScroll = 0.0f);

    /// Look up an action by name.  Returns nullptr if not found.
    const InputAction* FindAction(const std::string& name) const;

    // Direct device access
    const Keyboard& GetKeyboard() const noexcept { return _keyboard; }
    const Mouse&    GetMouse()    const noexcept { return _mouse; }
    const Gamepad&  GetGamepad(int index = 0) const noexcept;

    Keyboard& GetKeyboard() noexcept { return _keyboard; }
    Mouse&    GetMouse()    noexcept { return _mouse; }
    Gamepad&  GetGamepad(int index = 0) noexcept;

    /// Maximum gamepads tracked simultaneously.
    static constexpr int kMaxGamepads = 4;

private:
    Keyboard _keyboard;
    Mouse    _mouse;
    std::array<Gamepad, kMaxGamepads> _gamepads;
    std::vector<InputAction> _actions;
};

} // namespace Input

// ---------------------------------------------------------------------------
// InputComponent — attach to a player entity to record the current input
// state each frame so gameplay systems can query it through the ECS.
// ---------------------------------------------------------------------------

/// Per-entity snapshot of player input used by gameplay systems.
struct InputComponent : public IComponent {
    // Movement (normalised [-1,1])
    float moveForward  = 0.0f;
    float moveSideways = 0.0f;
    float moveUp       = 0.0f;

    // Look / aim (normalised [-1,1])
    float lookYaw   = 0.0f;
    float lookPitch = 0.0f;

    // Actions (polled each frame)
    bool fire           = false;
    bool fireSecondary  = false;
    bool interact       = false;
    bool toggleMap      = false;
    bool toggleInventory= false;

    /// Populate from the current InputSystem state.
    void UpdateFromInputSystem(const Input::InputSystem& sys);

    ComponentData Serialize()                     const;
    void          Deserialize(const ComponentData& data);
};

} // namespace subspace
