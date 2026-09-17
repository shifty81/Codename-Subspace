#pragma once

#include "ui/SubspaceUiFramework.h"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

namespace subspace {
class ShipyardOverlayLayoutStore {
public:
    static std::filesystem::path DefaultPath(){
        return std::filesystem::path("dist")/"user"/"shipyard_overlay_layout_v1.txt";
    }
    static bool Load(SubspaceDockWorkspace& current,const std::filesystem::path& path,
                     std::string* error=nullptr){
        auto fail=[&](const char* reason){if(error)*error=reason;return false;};
        std::ifstream source(path,std::ios::binary);
        if(!source)return fail("no saved overlay layout");
        const std::string contents((std::istreambuf_iterator<char>(source)),{});
        if(!source.eof()&&source.fail())return fail("cannot read overlay layout");
        if(contents.size()>131072)return fail("overlay layout exceeds size limit");
        SubspaceDockWorkspace restored;
        if(!SubspaceDockSystem::Deserialize(contents,restored,error))return false;
        if(restored.id!="shipyard"||restored.panels.size()!=current.panels.size()||
           restored.nodes.size()!=current.nodes.size())return fail("obsolete layout schema");
        // The saved file can change panel locations, visibility and ordering,
        // but it may not introduce unknown panels or strip core capabilities.
        for(const auto& canonical:current.panels){
            const auto* loaded=SubspaceDockSystem::FindPanel(restored,canonical.id);
            if(!loaded||loaded->floatable!=canonical.floatable||
               loaded->closable!=canonical.closable)return fail("incompatible panel catalog");
        }
        for(const auto& canonical:current.nodes)
            if(!SubspaceDockSystem::FindNode(restored,canonical.id))return fail("incompatible dock anchor");
        const auto* viewport=SubspaceDockSystem::FindPanel(restored,"viewport");
        if(!viewport||!viewport->visible)return fail("layout hides the canvas");
        current=std::move(restored);
        return true;
    }
    static bool Save(const SubspaceDockWorkspace& workspace,const std::filesystem::path& path,
                     std::string* error=nullptr){
        auto fail=[&](const char* reason){if(error)*error=reason;return false;};
        if(workspace.id!="shipyard"||!SubspaceDockSystem::Validate(workspace,error))
            return fail("invalid Shipyard layout cannot be saved");
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(),ec);
        if(ec)return fail("cannot create layout folder");
        auto temporary=path;temporary+=".tmp";
        auto backup=path;backup+=".bak";
        {
            std::ofstream output(temporary,std::ios::binary|std::ios::trunc);
            if(!output)return fail("cannot open layout temporary file");
            output<<SubspaceDockSystem::Serialize(workspace);
            output.flush();if(!output)return fail("cannot write layout temporary file");
        }
        std::filesystem::remove(backup,ec);ec.clear();
        const bool hadOld=std::filesystem::exists(path,ec);
        if(ec)return fail("cannot inspect saved layout");
        if(hadOld){
            std::filesystem::rename(path,backup,ec);
            if(ec)return fail("cannot back up saved layout");
        }
        std::filesystem::rename(temporary,path,ec);
        if(ec){
            if(hadOld){std::error_code rollback;
                std::filesystem::rename(backup,path,rollback);}
            return fail("cannot commit layout safely");
        }
        return true;
    }
};
} // namespace subspace
