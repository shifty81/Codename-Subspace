#pragma once
#include <cstdint>
#include <unordered_map>

namespace subspace {

using SpatialFrameId = std::uint64_t;
constexpr SpatialFrameId InvalidSpatialFrameId = 0;

struct Double3 { double x=0.0,y=0.0,z=0.0; };
struct DoubleQuat { double x=0.0,y=0.0,z=0.0,w=1.0; };

struct SpatialFrame {
    SpatialFrameId id = InvalidSpatialFrameId;
    SpatialFrameId parentId = InvalidSpatialFrameId;
    Double3 localPosition{};
    DoubleQuat localRotation{};
    Double3 linearVelocity{};
    Double3 angularVelocity{};
};

struct SpatialKinematicState {
    SpatialFrameId frameId = InvalidSpatialFrameId;
    Double3 localPosition{};
    Double3 localVelocity{};
};

class SpatialFrameSystem {
public:
    bool Upsert(const SpatialFrame& frame);
    bool Remove(SpatialFrameId id);
    const SpatialFrame* Find(SpatialFrameId id) const;

    Double3 ToParentPoint(SpatialFrameId id,const Double3& local) const;
    Double3 ToWorldPoint(SpatialFrameId id,const Double3& local) const;
    Double3 ToWorldVelocity(SpatialFrameId id,const Double3& localPoint,const Double3& localVelocity) const;
    SpatialKinematicState ReparentPreservingWorld(const SpatialKinematicState& state,SpatialFrameId newParent) const;

private:
    std::unordered_map<SpatialFrameId,SpatialFrame> frames_;
};

} // namespace subspace
