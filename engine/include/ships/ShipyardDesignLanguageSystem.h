#pragma once
#include <string>
#include <vector>
namespace subspace {
struct ShipyardDesignLanguageReference { std::string name; float width=1,length=1,height=1; };
class ShipyardDesignLanguageSystem {
public:
    static const std::vector<ShipyardDesignLanguageReference>& References();
    // Reference ships contribute only broad ratio/orientation priors. This
    // score never selects modules or copies topology/transforms from an example.
    static float Score(float width,float length,float height,bool hasCommand,bool hasAftDrive,bool balanced);
};
}
