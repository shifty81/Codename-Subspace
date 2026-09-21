#pragma once
#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace subspace {
// Deterministic cache identity, not a persisted asset ID. Include every
// assembly transform and attachment field the carve/link pipeline consumes;
// include catalog dimensions, semantic roles, and socket frames so authored
// overrides invalidate the derived preview without re-planning every frame.
struct StudioInteriorPreviewKey {
    static std::string Compute(const std::vector<ShipyardModuleRecord>& catalog,
                               const ProceduralShipVisualRecipe& recipe){
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out<<std::setprecision(std::numeric_limits<float>::max_digits10);
        auto id=[&](const std::string& value){out<<value.size()<<':'<<value<<';';};
        out<<recipe.widthScale<<','<<recipe.lengthScale<<','
           <<recipe.modules.size()<<','<<recipe.attachments.size()<<';';
        for(const auto& m:recipe.modules){
            id(m.moduleId);
            out<<m.x<<','<<m.y<<','<<m.z<<','
               <<m.scaleX<<','<<m.scaleY<<','<<m.scaleZ<<','
               <<m.yawDegrees<<','<<m.pitchDegrees<<','<<m.rollDegrees<<','
               <<m.mirrorX<<m.mirrorY<<m.mirrorZ<<';';
        }
        for(const auto& a:recipe.attachments){
            out<<a.parentModuleIndex<<','<<a.childModuleIndex<<',';
            id(a.parentSocket);id(a.childSocket);
            out<<a.measuredGap<<','<<a.certified<<';';
        }
        std::unordered_set<std::string> used;
        for(const auto& module:recipe.modules)used.insert(module.moduleId);
        out<<used.size()<<';';
        for(const auto& record:catalog){
            if(!used.count(record.source.moduleId))continue;
            id(record.source.moduleId);
            out<<record.source.halfWidth<<','<<record.source.halfLength<<','
               <<record.source.halfHeight<<','<<static_cast<int>(record.partRole)<<','
               <<static_cast<int>(record.semantic)<<','<<static_cast<int>(record.moduleClass)<<','
               <<record.functional<<record.surfaceOnly<<',';
            id(record.placementRole);
            out<<record.sockets.size()<<';';
            for(const auto& socket:record.sockets){
                id(socket.name);id(socket.type);
                out<<socket.x<<','<<socket.y<<','<<socket.z<<','
                   <<socket.dirX<<','<<socket.dirY<<','<<socket.dirZ<<','
                   <<socket.upX<<','<<socket.upY<<','<<socket.upZ<<','
                   <<socket.insertionDepth<<','<<socket.manualOverride<<';';
            }
        }
        return out.str();
    }
};
} // namespace subspace
