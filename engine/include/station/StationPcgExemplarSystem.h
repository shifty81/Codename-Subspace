#pragma once

#include "station/StationDesignDnaSystem.h"

#include <string>
#include <vector>

namespace subspace {

class StationPcgExemplarSystem {
public:
    bool Add(StationDesignExemplar exemplar);
    bool Remove(const std::string& name);
    const StationDesignExemplar* Find(const std::string& name) const;
    std::vector<StationDesignExemplar> ForArchetype(StationArchetype archetype) const;
    StationDesignFamily Compile(const std::string& familyId, StationArchetype archetype) const;
    const std::vector<StationDesignExemplar>& All() const { return exemplars_; }
private:
    std::vector<StationDesignExemplar> exemplars_;
};

} // namespace subspace
