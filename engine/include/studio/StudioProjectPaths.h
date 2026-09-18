#pragma once
#include <filesystem>

namespace subspace {
// Resolve the project-owned data root without relying on a specific launcher,
// the user's home directory, or a machine-wide ForgePY install.
struct StudioProjectPaths {
    static std::filesystem::path Root(){
        std::error_code ec;
        auto path=std::filesystem::current_path(ec);
        if(ec)return {};
        for(int i=0;i<8&&!path.empty();++i){
            ec.clear();
            if(std::filesystem::exists(path/"SubspaceTools.ps1",ec) && !ec &&
               std::filesystem::is_directory(path/"engine",ec) && !ec)return path;
            const auto parent=path.parent_path();
            if(parent==path)break;
            path=parent;
        }
        return {};
    }
    static std::filesystem::path Blueprints(){const auto root=Root();return root.empty()?root:root/"dist"/"blueprints";}
    static std::filesystem::path SocketOverrides(){const auto root=Root();return root.empty()?root:root/"content"/"authoring"/"shipyard_socket_overrides.subspace_socket_overrides";}
    static std::filesystem::path DefinitionOverrides(){const auto root=Root();return root.empty()?root:root/"content"/"authoring"/"shipyard_definition_overrides.subspace_definition_overrides";}
};
}
