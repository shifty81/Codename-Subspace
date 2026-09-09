#include "station/StationDesignDnaSystem.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& c,const std::string& id){auto it=std::find_if(c.begin(),c.end(),[&](const auto&r){return r.source.moduleId==id;});return it==c.end()?nullptr:&*it;}
bool Structural(const ShipyardModuleRecord&r){return r.partRole==ShipyardPartRole::PrimaryHull||r.partRole==ShipyardPartRole::StructuralFrame||r.partRole==ShipyardPartRole::StructuralAttachment||r.partRole==ShipyardPartRole::StructuralBrace||r.partRole==ShipyardPartRole::StructuralBlock||r.semantic==ShipyardModuleSemantic::StructuralFrame||r.semantic==ShipyardModuleSemantic::HullMid;}
}
StationDesignDna StationDesignDnaSystem::Extract(const StationKitbashVisualRecipe& recipe,const std::vector<ShipyardModuleRecord>& catalog){
    StationDesignDna d;d.id=recipe.identity+"_DNA";d.grammarId=recipe.grammarId;d.archetype=recipe.archetype;d.moduleCount=static_cast<int>(recipe.modules.size());d.attachmentCount=static_cast<int>(recipe.attachments.size());
    float maxX=0,maxY=0,maxZ=0;int structural=0,radial=0;std::set<std::string>vocab;std::vector<float>angles;
    for(std::size_t i=0;i<recipe.modules.size();++i){const auto&p=recipe.modules[i];const auto*r=Find(catalog,p.moduleId);float hx=.5f,hy=.5f,hz=.5f;if(r){hx=r->source.halfWidth*std::abs(p.scaleX);hy=r->source.halfLength*std::abs(p.scaleY);hz=r->source.halfHeight*std::abs(p.scaleZ);structural+=Structural(*r);}maxX=std::max(maxX,std::abs(p.x)+hx);maxY=std::max(maxY,std::abs(p.y)+hy);maxZ=std::max(maxZ,std::abs(p.z)+hz);if(std::sqrt(p.x*p.x+p.y*p.y)>.25f){angles.push_back(std::atan2(p.y,p.x));++radial;}if(i<recipe.logicalPieceIds.size())vocab.insert(recipe.logicalPieceIds[i]);}
    d.halfWidth=maxX;d.halfLength=maxY;d.halfHeight=maxZ;d.branchLikeModules=radial;d.structuralRatio=d.moduleCount?float(structural)/float(d.moduleCount):0.0f;
    d.radiality=d.moduleCount?std::clamp(float(radial)/float(d.moduleCount),0.0f,1.0f):0.0f;
    int mirrored=0,considered=0;for(const auto&p:recipe.modules){if(std::abs(p.x)<.2f)continue;++considered;for(const auto&q:recipe.modules){if(std::abs(q.x+p.x)<.30f&&std::abs(q.y-p.y)<.30f){++mirrored;break;}}}d.symmetry=considered?std::clamp(float(mirrored)/float(considered),0.0f,1.0f):1.0f;
    d.pieceVocabulary.assign(vocab.begin(),vocab.end());return d;
}
StationDesignFamily StationDesignDnaSystem::CompileFamily(const std::string&id,StationArchetype archetype,const std::vector<StationDesignExemplar>& exemplars){StationDesignFamily f;f.id=id;f.archetype=archetype;float total=0;std::set<std::string>vocab;for(const auto&e:exemplars){if(!e.approved||e.dna.archetype!=archetype||e.weight<=0)continue;const float w=e.weight;total+=w;++f.exemplarCount;f.averageRadiality+=e.dna.radiality*w;f.averageSymmetry+=e.dna.symmetry*w;f.averageStructuralRatio+=e.dna.structuralRatio*w;vocab.insert(e.dna.pieceVocabulary.begin(),e.dna.pieceVocabulary.end());}if(total>0){f.averageRadiality/=total;f.averageSymmetry/=total;f.averageStructuralRatio/=total;}f.vocabulary.assign(vocab.begin(),vocab.end());return f;}
} // namespace subspace
