#include "ship_editor/ShipyardUniversalSearchSystem.h"

#include "content/ShipyardPartTaxonomySystem.h"

#include <algorithm>
#include <cctype>

namespace subspace {
namespace {
std::string Lower(std::string value){std::transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});return value;}
float Score(std::string haystack,const std::string& needle,float base){
    haystack=Lower(std::move(haystack));if(needle.empty())return base;
    const auto pos=haystack.find(needle);if(pos==std::string::npos)return -1.0f;
    return base+(pos==0?8.0f:std::max(0.0f,4.0f-static_cast<float>(pos)*.05f));
}
}

std::vector<ShipyardSearchResult> ShipyardUniversalSearchSystem::Search(std::string query,
                                                                        const ShipyardCommandSystem& commands,
                                                                        const std::vector<ShipyardModuleRecord>& catalog,
                                                                        const ShipyardPanelModel& panelModel,
                                                                        const std::vector<EditorValidationMessage>& validation,
                                                                        bool includeAdvanced,
                                                                        std::size_t limit){
    std::vector<ShipyardSearchResult> out;const auto needle=Lower(query);
    for(const auto& command:commands.All(includeAdvanced)){
        const float s=Score(command.label+" "+command.id+" "+command.category+" "+command.defaultShortcut,needle,50.0f);if(s<0)continue;
        out.push_back({command.advanced?ShipyardSearchGroup::Developer:ShipyardSearchGroup::Command,command.id,command.label,command.category,s,command.advanced});
    }
    for(const auto& asset:catalog){
        const auto label=ShipyardPartTaxonomySystem::DisplayName(asset);const float s=Score(label+" "+asset.source.moduleId+" "+ShipyardModuleSystem::SemanticName(asset.semantic),needle,35.0f);if(s<0)continue;
        out.push_back({ShipyardSearchGroup::Asset,asset.source.moduleId,label,ShipyardPartTaxonomySystem::CategoryName(asset.moduleClass),s,false});
    }
    for(const auto& node:panelModel.outliner){const float s=Score(node.label+" "+node.kind,needle,42.0f);if(s>=0)out.push_back({ShipyardSearchGroup::ShipObject,ShipyardStableIdSystem::ToString(node.id),node.label,node.kind,s,false});}
    for(const auto& section:panelModel.properties)for(const auto& property:section.properties){const float s=Score(property.label+" "+property.id+" "+property.value,needle,30.0f);if(s>=0)out.push_back({ShipyardSearchGroup::Property,property.id,property.label,section.title+" / "+property.value,s,property.advanced});}
    for(const auto& panel:ShipyardProfessionalUiSystem::Panels()){if(panel.advanced&&!includeAdvanced)continue;const float s=Score(panel.title+" "+panel.id,needle,38.0f);if(s>=0)out.push_back({panel.advanced?ShipyardSearchGroup::Developer:ShipyardSearchGroup::Panel,"panel."+panel.id,panel.title,"Panel",s,panel.advanced});}
    for(const auto& message:validation){const float s=Score(message.code+" "+message.message+" "+message.targetId,needle,45.0f);if(s>=0)out.push_back({ShipyardSearchGroup::Validation,message.code,message.message,message.targetId,s,false});}
    std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){if(a.score!=b.score)return a.score>b.score;if(a.group!=b.group)return static_cast<int>(a.group)<static_cast<int>(b.group);return a.label<b.label;});
    if(out.size()>limit)out.resize(limit);
    return out;
}

const char* ShipyardUniversalSearchSystem::GroupName(ShipyardSearchGroup group){
    switch(group){case ShipyardSearchGroup::Command:return "COMMANDS";case ShipyardSearchGroup::Asset:return "ASSETS";case ShipyardSearchGroup::ShipObject:return "SHIP OBJECTS";case ShipyardSearchGroup::Property:return "PROPERTIES";case ShipyardSearchGroup::Panel:return "PANELS";case ShipyardSearchGroup::Validation:return "VALIDATION";case ShipyardSearchGroup::Developer:return "DEVELOPER";}return "SEARCH";
}
}
