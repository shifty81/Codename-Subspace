#pragma once
#include "input/InputState.h"
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
struct InputBinding { InputAction action=InputAction::ThrustForward; std::string key; };
class InputBindingProfile { public: static InputBindingProfile Defaults(); bool Rebind(InputAction action,const std::string& key); std::string KeyFor(InputAction action) const; bool HasConflicts() const; std::vector<InputBinding> Bindings() const; private: std::unordered_map<std::size_t,std::string> bindings_; };
}
