#pragma once

#include <array>
#include <cstddef>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace subspace {

// Presentation metadata only. Transform authority stays in ShipyardBuilderSystem
// and StudioTransformReadout; the HUD and future editable inspector can share
// these labels/units without creating an independent transform or unit system.
enum class StudioMeasurementKind { Position, Rotation, Scale, NominalDimensions };

class StudioMeasurementFormat {
public:
    static const char* Label(StudioMeasurementKind kind) noexcept {
        switch(kind) {
        case StudioMeasurementKind::Position: return "POS";
        case StudioMeasurementKind::Rotation: return "ROT";
        case StudioMeasurementKind::Scale: return "SCL";
        case StudioMeasurementKind::NominalDimensions: return "DIM";
        }
        return "---";
    }
    static const char* AxisLabel(StudioMeasurementKind kind,int axis) noexcept {
        if(axis<0||axis>2)return "?";
        static constexpr std::array<const char*,3> xyz{{"X","Y","Z"}};
        static constexpr std::array<const char*,3> dimensions{{"W","L","H"}};
        return kind==StudioMeasurementKind::NominalDimensions?
            dimensions[static_cast<std::size_t>(axis)]:xyz[static_cast<std::size_t>(axis)];
    }
    static bool IsMeters(StudioMeasurementKind kind) noexcept {
        return kind==StudioMeasurementKind::Position||kind==StudioMeasurementKind::NominalDimensions;
    }
    static bool IsDegrees(StudioMeasurementKind kind) noexcept {
        return kind==StudioMeasurementKind::Rotation;
    }
    static bool IsPercent(StudioMeasurementKind kind) noexcept {
        return kind==StudioMeasurementKind::Scale;
    }
    static std::string Compact(float value,int decimals) {
        if(!std::isfinite(value)||std::fabs(value)>9999.0f||decimals<0||decimals>3)
            return "--"; // A clipped number must not masquerade as its true value.
        if(std::fabs(value)<.5f*std::pow(10.0f,-decimals))value=0;
        std::ostringstream result;
        result<<std::fixed<<std::setprecision(decimals)<<value;
        return result.str();
    }
};

} // namespace subspace
