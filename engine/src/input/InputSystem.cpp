#include "input/InputSystem.h"

#include <algorithm>

namespace subspace {
namespace Input {

// ---------------------------------------------------------------------------
// InputAction
// ---------------------------------------------------------------------------

bool InputAction::GetBool(const Keyboard& kb) const {
    for (const KeyCode key : bindings)
        if (kb.IsKeyDown(key)) return true;
    return false;
}

float InputAction::GetFloat(const Keyboard& kb) const {
    return GetBool(kb) ? 1.0f : 0.0f;
}

Vector2 InputAction::GetVector2(const Keyboard& kb) const {
    Vector2 result{};
    const Vector2 dirs[4] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (std::size_t i = 0; i < bindings.size() && i < 4; ++i)
        if (kb.IsKeyDown(bindings[i]))
            result = result + dirs[i];
    return result;
}

// ---------------------------------------------------------------------------
// InputSystem
// ---------------------------------------------------------------------------

InputSystem::InputSystem() : SystemBase("InputSystem") {}

void InputSystem::RegisterAction(InputAction action) {
    for (auto& a : _actions) {
        if (a.name == action.name) { a = std::move(action); return; }
    }
    _actions.push_back(std::move(action));
}

void InputSystem::BindKey(const std::string& actionName, KeyCode key) {
    for (auto& a : _actions) {
        if (a.name == actionName) { a.bindings.push_back(key); return; }
    }
}

void InputSystem::PollDevices(
    const std::array<bool, static_cast<std::size_t>(KeyCode::Count)>& keys,
    float mouseX, float mouseY, uint8_t mouseButtons, float mouseScroll) {
    _keyboard.Update(keys);
    _mouse.Update(mouseX, mouseY, mouseButtons, mouseScroll);
}

const InputAction* InputSystem::FindAction(const std::string& name) const {
    for (const auto& a : _actions)
        if (a.name == name) return &a;
    return nullptr;
}

const Gamepad& InputSystem::GetGamepad(int index) const noexcept {
    int idx = (index >= 0 && index < kMaxGamepads) ? index : 0;
    return _gamepads[static_cast<std::size_t>(idx)];
}

Gamepad& InputSystem::GetGamepad(int index) noexcept {
    int idx = (index >= 0 && index < kMaxGamepads) ? index : 0;
    return _gamepads[static_cast<std::size_t>(idx)];
}

} // namespace Input

// ---------------------------------------------------------------------------
// InputComponent
// ---------------------------------------------------------------------------

void InputComponent::UpdateFromInputSystem(const Input::InputSystem& sys) {
    const auto& kb = sys.GetKeyboard();
    const auto& mo = sys.GetMouse();

    // Default WASD + QE for 3-D movement
    moveForward  = (kb.IsKeyDown(Input::KeyCode::W)      ? 1.0f : 0.0f)
                 - (kb.IsKeyDown(Input::KeyCode::S)      ? 1.0f : 0.0f);
    moveSideways = (kb.IsKeyDown(Input::KeyCode::D)      ? 1.0f : 0.0f)
                 - (kb.IsKeyDown(Input::KeyCode::A)      ? 1.0f : 0.0f);
    moveUp       = (kb.IsKeyDown(Input::KeyCode::Space)  ? 1.0f : 0.0f)
                 - (kb.IsKeyDown(Input::KeyCode::LeftCtrl)? 1.0f : 0.0f);

    lookYaw   = mo.GetDeltaX();
    lookPitch = mo.GetDeltaY();

    fire            = mo.IsButtonDown(Input::MouseButton::Left);
    fireSecondary   = mo.IsButtonDown(Input::MouseButton::Right);
    interact        = kb.IsKeyDown(Input::KeyCode::F1);
    toggleMap       = kb.IsKeyDown(Input::KeyCode::M);
    toggleInventory = kb.IsKeyDown(Input::KeyCode::I);
}

ComponentData InputComponent::Serialize() const {
    ComponentData d;
    d.componentType = "InputComponent";
    d.data["moveForward"]  = std::to_string(moveForward);
    d.data["moveSideways"] = std::to_string(moveSideways);
    d.data["moveUp"]       = std::to_string(moveUp);
    return d;
}

void InputComponent::Deserialize(const ComponentData& data) {
    if (data.data.count("moveForward"))
        moveForward  = std::stof(data.data.at("moveForward"));
    if (data.data.count("moveSideways"))
        moveSideways = std::stof(data.data.at("moveSideways"));
    if (data.data.count("moveUp"))
        moveUp       = std::stof(data.data.at("moveUp"));
}

} // namespace subspace
