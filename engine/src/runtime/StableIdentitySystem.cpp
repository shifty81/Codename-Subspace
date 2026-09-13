#include "runtime/StableIdentitySystem.h"
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {
std::uint64_t Fnv1a64(const std::string& text, std::uint64_t seed) {
    std::uint64_t h = seed;
    for (unsigned char c : text) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}
}

PersistentEntityId StableIdentitySystem::Deterministic(PersistentEntityKind kind,
                                                        const std::string& authority,
                                                        const std::string& stableKey,
                                                        std::uint64_t generation) {
    const std::string canonical = std::to_string(static_cast<unsigned>(kind)) + "|" +
                                  authority + "|" + stableKey + "|" + std::to_string(generation);
    PersistentEntityId id;
    id.high = Fnv1a64(canonical, 1469598103934665603ull);
    id.low = Fnv1a64(canonical, 1099511628211ull ^ 0x9e3779b97f4a7c15ull);
    if (!id.IsValid()) id.low = 1;
    return id;
}

std::string StableIdentitySystem::ToString(const PersistentEntityId& id) {
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << id.high
        << std::setw(16) << id.low;
    return out.str();
}

} // namespace subspace
