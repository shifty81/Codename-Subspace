#pragma once

#include "station/StationDesignDnaSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct StationBlueprintDocument {
    std::string blueprintId = "station_blueprint";
    std::string name = "Untitled Station";
    std::string author = "PLAYER";
    int revision = 1;
    StationKitbashVisualRecipe recipe{};
    StationDesignDna designDna{};
    std::vector<std::string> tags;
};

class StationBlueprintLibrarySystem {
public:
    static bool Save(const StationBlueprintDocument& blueprint,const std::string& path,std::string* error=nullptr);
    static bool Load(const std::string& path,StationBlueprintDocument& blueprint,std::string* error=nullptr);
    static std::string CanonicalId(const StationBlueprintDocument& blueprint);
};

} // namespace subspace
