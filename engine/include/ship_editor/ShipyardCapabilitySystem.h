#pragma once

namespace subspace {

enum class ShipyardAccessMode {
    PlayerDocked,
    MainMenuStudio,
    RuntimeDeveloper
};

struct ShipyardCapabilityProfile {
    bool build = true;
    bool appearance = true;
    bool systems = true;
    bool model = false;
    bool pcgStudio = false;
    bool world = false;
    bool interior = false;
    bool character = false;
    bool devWorld = false;
    bool sockets = false;
    bool rawAuthoring = false;
    bool publishCanonicalAsset = false;
    bool playFromHere = false;
    bool modifySourceClassification = false;
};

class ShipyardCapabilitySystem {
public:
    static ShipyardCapabilityProfile For(ShipyardAccessMode mode);
    static const char* ModeName(ShipyardAccessMode mode);
};

} // namespace subspace
