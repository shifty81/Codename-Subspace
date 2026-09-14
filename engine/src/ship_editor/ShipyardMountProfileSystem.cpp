#include "ship_editor/ShipyardMountProfileSystem.h"

#include <algorithm>
#include <cctype>

namespace subspace {
namespace {

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

ShipyardMountSurface Convert(const char* face, const VisualModuleSurfaceContact& contact) {
    return {face, contact.point, contact.normal, contact.supportingArea, contact.confidence, contact.valid};
}

std::vector<ShipyardMountSurface> AllSurfaces(const ShipyardModuleRecord& record) {
    return {
        Convert("forward", record.source.forwardSurface),
        Convert("aft", record.source.aftSurface),
        Convert("port", record.source.portSurface),
        Convert("starboard", record.source.starboardSurface),
        Convert("dorsal", record.source.dorsalSurface),
        Convert("ventral", record.source.ventralSurface)
    };
}

} // namespace

ShipyardMountSurface ShipyardMountProfileSystem::SurfaceForFace(const ShipyardModuleRecord& record,
                                                                 std::string face) {
    face = Lower(std::move(face));
    for (const auto& surface : AllSurfaces(record)) if (surface.face == face) return surface;
    if (face == "lateral") {
        const auto port = Convert("port", record.source.portSurface);
        const auto starboard = Convert("starboard", record.source.starboardSurface);
        return port.supportingArea >= starboard.supportingArea ? port : starboard;
    }
    return {};
}

ShipyardMountSurface ShipyardMountProfileSystem::LargestFlatSurface(const ShipyardModuleRecord& record) {
    auto surfaces = AllSurfaces(record);
    const auto it = std::max_element(surfaces.begin(), surfaces.end(), [](const auto& a, const auto& b) {
        const float aa = a.valid ? a.supportingArea : -1.0f;
        const float bb = b.valid ? b.supportingArea : -1.0f;
        return aa < bb;
    });
    return it == surfaces.end() ? ShipyardMountSurface{} : *it;
}

bool ShipyardMountProfileSystem::IsBroadRootStructuralPart(const ShipyardModuleRecord& record) {
    const auto id = Lower(record.source.moduleId);
    return id.find("enginestrutfoot") != std::string::npos ||
           (record.semantic == ShipyardModuleSemantic::StructuralFrame && record.preferredMountFace.empty());
}

ShipyardMountProfile ShipyardMountProfileSystem::Build(const ShipyardModuleRecord& record) {
    ShipyardMountProfile out;
    out.moduleId = record.source.moduleId;
    out.placementRole = record.placementRole;
    out.generatorEligible = record.generatorEligible;
    out.pairedPlacement = record.pairedPlacement;

    std::string preferred = Lower(record.preferredMountFace);
    const bool autoFace = preferred.empty() || preferred == "auto";

    // Structural feet/brackets with a broad planar base should root from the
    // actual geometry-supported face, not an arbitrary nearest socket. This is
    // the normalized authority for the engine-strut-foot module called out in
    // visual review and applies safely to similar future structural feet.
    if (autoFace && IsBroadRootStructuralPart(record)) {
        out.primaryRoot = LargestFlatSurface(record);
        out.provenance = "geometry-largest-flat-root";
        if (out.primaryRoot.valid) out.primaryRoot.confidence = std::max(out.primaryRoot.confidence, 0.98f);
    } else if (!autoFace) {
        out.primaryRoot = SurfaceForFace(record, preferred);
        out.provenance = "authored-preferred-mount-face";
    } else {
        out.primaryRoot = LargestFlatSurface(record);
        out.provenance = "geometry-largest-flat-fallback";
    }

    for (const auto& surface : AllSurfaces(record)) {
        if (!surface.valid || surface.face == out.primaryRoot.face) continue;
        out.alternates.push_back(surface);
    }
    std::sort(out.alternates.begin(), out.alternates.end(), [](const auto& a, const auto& b) {
        return a.supportingArea > b.supportingArea;
    });

    float deepest = 0.0f;
    for (const auto& socket : record.sockets) deepest = std::max(deepest, socket.insertionDepth);
    out.insertionDepthMeters = std::max(0.01f, deepest);
    out.minimumClearanceMeters = record.surfaceOnly ? 0.04f : 0.10f;
    out.valid = out.primaryRoot.valid;
    return out;
}

} // namespace subspace
