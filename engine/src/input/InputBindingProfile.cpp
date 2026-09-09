#include "input/InputBindingProfile.h"
#include <set>

namespace subspace {

InputBindingProfile InputBindingProfile::Defaults(){
    InputBindingProfile p;
    p.Rebind(InputAction::ThrustForward,"W");
    p.Rebind(InputAction::ThrustReverse,"S");
    p.Rebind(InputAction::StrafeLeft,"A");
    p.Rebind(InputAction::StrafeRight,"D");
    p.Rebind(InputAction::TurnLeft,"Q");
    p.Rebind(InputAction::TurnRight,"E");
    p.Rebind(InputAction::ToggleFlightMode,"TAB");
    p.Rebind(InputAction::Boost,"SHIFT");
    p.Rebind(InputAction::RequestDock,"J");
    p.Rebind(InputAction::OpenGalaxyMap,"M");
    p.Rebind(InputAction::OpenSystemMap,"N");
    p.Rebind(InputAction::ToggleShipInspection,"F6");
    return p;
}

bool InputBindingProfile::Rebind(InputAction a,const std::string&k){if(k.empty())return false;bindings_[static_cast<std::size_t>(a)]=k;return true;}
std::string InputBindingProfile::KeyFor(InputAction a) const {auto i=bindings_.find(static_cast<std::size_t>(a));return i==bindings_.end()?std::string{}:i->second;}
bool InputBindingProfile::HasConflicts() const {std::set<std::string>s;for(const auto&kv:bindings_)if(!s.insert(kv.second).second)return true;return false;}
std::vector<InputBinding> InputBindingProfile::Bindings() const {std::vector<InputBinding>o;for(const auto&kv:bindings_)o.push_back({static_cast<InputAction>(kv.first),kv.second});return o;}

} // namespace subspace
