#pragma once

#include <string>
#include <vector>

namespace subspace {

struct VerticalSliceCheckpoint { std::string id; bool satisfied=false; };
class SandboxVerticalSliceSystem {
public:
    std::vector<VerticalSliceCheckpoint> Pass220Checklist(bool mainMenu,bool starterShip,bool hangar,bool undocked,bool scannedSite,bool gatheredResources,bool fieldCrafted,bool marketUsed,bool captainHired,bool saveReady) const;
    bool Complete(const std::vector<VerticalSliceCheckpoint>& checkpoints) const;
};

} // namespace subspace
