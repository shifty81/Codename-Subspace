#pragma once

#include "content/ShipyardModuleSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardAssetBrowserDensity { Compact, Comfortable, Large };

struct ShipyardAssetBrowserSavedFilter {
    std::string id;
    std::string label;
    std::string query;
    std::vector<std::string> requiredTags;
    bool compatibleOnly = false;
    bool favoritesOnly = false;
};

struct ShipyardAssetBrowserState {
    std::string search;
    std::vector<std::string> requiredTags;
    std::vector<std::string> favorites;
    std::vector<std::string> recent;
    std::vector<ShipyardAssetBrowserSavedFilter> savedFilters;
    ShipyardAssetBrowserDensity density = ShipyardAssetBrowserDensity::Comfortable;
    float thumbnailScale = 1.0f;
    bool compatibleOnly = false;
    bool showIncompatible = true;
    bool favoritesOnly = false;
    bool generatorEligibleOnly = false;
};

struct ShipyardAssetBrowserItem {
    std::string moduleId;
    std::string label;
    std::string category;
    std::string semantic;
    std::string size;
    std::vector<std::string> tags;
    bool favorite = false;
    bool compatible = true;
    bool generatorEligible = false;
    float score = 0.0f;
};

class ShipyardAssetBrowserSystem {
public:
    static ShipyardAssetBrowserState DefaultState();
    static std::vector<std::string> TagsFor(const ShipyardModuleRecord& record);
    static bool CompatibleWith(const ShipyardModuleRecord& candidate,
                               const ShipyardModuleRecord* selectedParent);
    static std::vector<ShipyardAssetBrowserItem> Query(const std::vector<ShipyardModuleRecord>& catalog,
                                                       const ShipyardAssetBrowserState& state,
                                                       const ShipyardModuleRecord* selectedParent = nullptr);
    static bool ToggleFavorite(ShipyardAssetBrowserState& state, const std::string& moduleId);
    static void MarkRecent(ShipyardAssetBrowserState& state, const std::string& moduleId, std::size_t limit = 12);
    static bool ApplySavedFilter(ShipyardAssetBrowserState& state, const std::string& filterId);
    static float ClampThumbnailScale(float value);
};

} // namespace subspace
