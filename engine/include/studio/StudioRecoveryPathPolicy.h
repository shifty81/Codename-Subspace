#pragma once
#include <filesystem>
#include <string>

namespace subspace {
struct StudioRecoveryPathPolicy {
    // The recovery lane never points at an opened blueprint's canonical path.
    static std::filesystem::path Candidate(const std::filesystem::path& blueprintDirectory,
                                           long long tick,unsigned sequence){
        if(blueprintDirectory.empty())return {};
        return blueprintDirectory/"recovery"/
            ("studio_exit_recovery_"+std::to_string(tick)+"_"+
             std::to_string(sequence)+".subspace_ship");
    }
    static constexpr bool NeedsRecovery(bool dirty) noexcept {return dirty;}
};
}
