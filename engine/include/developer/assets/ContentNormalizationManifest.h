#pragma once

#include <string>
#include <vector>

namespace subspace {

struct ContentNormalizationMove {
    std::string from;
    std::string to;
    std::string reason;
    bool destructive = false;
};

struct ContentNormalizationManifest {
    std::vector<ContentNormalizationMove> plannedMoves;
    std::vector<std::string> rootFilesToReview;
    std::vector<std::string> warnings;

    bool HasDestructiveMoves() const;
    std::size_t MoveCount() const { return plannedMoves.size(); }
    std::size_t ReviewCount() const { return rootFilesToReview.size(); }
};

class ContentNormalizationManifestBuilder {
public:
    ContentNormalizationManifest BuildDefaultPlan() const;
};

} // namespace subspace
