#include "runtime/AssemblyRuntimeProductSystem.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace subspace {
namespace {
const ShipyardModuleRecord* FindRecord(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){for(const auto& r:catalog)if(r.source.moduleId==id)return &r;return nullptr;}
bool HasTag(const BuildElement& e,const char* tag){return std::find(e.capabilityTags.begin(),e.capabilityTags.end(),tag)!=e.capabilityTags.end();}
const BuildElement* FindElement(const AssemblyDefinition& a,const std::string& id){for(const auto& e:a.elements)if(e.id==id)return &e;return nullptr;}
std::uint64_t Hash(std::uint64_t h,const std::string& s){for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}
std::string Hex(std::uint64_t v){std::ostringstream o;o<<std::hex<<std::setfill('0')<<std::setw(16)<<v;return o.str();}
Double3 Mid(Double3 a,Double3 b){return {(a.x+b.x)*.5,(a.y+b.y)*.5,(a.z+b.z)*.5};}
}

AssemblyRuntimeProducts AssemblyRuntimeProductSystem::Compile(const AssemblyDefinition& assembly,
                                                               const std::vector<ShipyardModuleRecord>& catalog,
                                                               const std::vector<std::string>& requiredCapabilities){
    AssemblyRuntimeProducts out;AssemblyConstructionSystem editor;std::string error;
    if(!editor.Begin(assembly,&error)){out.warnings.push_back(error);return out;}
    out.source=editor.Compile(requiredCapabilities);if(!out.source.valid){out.warnings.push_back("canonical assembly validation failed before runtime-product compilation");return out;}
    double minx=std::numeric_limits<double>::infinity(),miny=minx,minz=minx,maxx=-minx,maxy=-minx,maxz=-minx;
    double mass=0.0,cx=0.0,cy=0.0,cz=0.0;
    for(const auto& e:assembly.elements){
        const auto* r=FindRecord(catalog,e.definitionId);if(!r){out.warnings.push_back("missing catalog geometry for "+e.definitionId);continue;}
        out.renderInstances.push_back({e.id,e.definitionId,e.transform});
        const Double3 half{std::fabs(e.transform.scale.x)*r->source.halfWidth,std::fabs(e.transform.scale.y)*r->source.halfLength,std::fabs(e.transform.scale.z)*r->source.halfHeight};
        out.collisionProxies.push_back({e.id,e.transform.position,half,e.transform.rotation});
        minx=std::min(minx,e.transform.position.x-half.x);miny=std::min(miny,e.transform.position.y-half.y);minz=std::min(minz,e.transform.position.z-half.z);
        maxx=std::max(maxx,e.transform.position.x+half.x);maxy=std::max(maxy,e.transform.position.y+half.y);maxz=std::max(maxz,e.transform.position.z+half.z);
        mass+=e.massKg;cx+=e.transform.position.x*e.massKg;cy+=e.transform.position.y*e.massKg;cz+=e.transform.position.z*e.massKg;
        const bool habitable=HasTag(e,"interior_candidate")&&!HasTag(e,"propulsion")&&!HasTag(e,"aero_surface");
        if(habitable) out.interiorVolumes.push_back({e.id,e.transform.position,half,true});
        if(HasTag(e,"propulsion")||HasTag(e,"maneuvering")) out.propulsionPorts.push_back({e.id,{e.transform.position.x,e.transform.position.y-half.y,e.transform.position.z},{0,-1,0},std::max(.05,std::min(half.x,half.z)*.2)});
    }
    for(const auto& a:assembly.attachments){
        const auto* ea=FindElement(assembly,a.aElementId);const auto* eb=FindElement(assembly,a.bElementId);if(!ea||!eb)continue;
        const auto midpoint=Mid(ea->transform.position,eb->transform.position);out.portals.push_back({a.aElementId,a.bElementId,midpoint,a.structural});out.navAnchors.push_back({"portal-"+a.aElementId+"-"+a.bElementId,midpoint});
    }
    if(out.renderInstances.empty()){out.boundsMin={};out.boundsMax={};}else{out.boundsMin={minx,miny,minz};out.boundsMax={maxx,maxy,maxz};}
    if(mass>0.0) out.centerOfMass={cx/mass,cy/mass,cz/mass};
    std::uint64_t h=1469598103934665603ull;h=Hash(h,out.source.fingerprint);h=Hash(h,std::to_string(out.renderInstances.size()));h=Hash(h,std::to_string(out.collisionProxies.size()));h=Hash(h,std::to_string(out.interiorVolumes.size()));h=Hash(h,std::to_string(out.portals.size()));
    for(const auto& r:out.renderInstances)h=Hash(h,r.elementId+"|"+r.moduleId);
    out.fingerprint=Hex(h);out.valid=out.source.valid&&out.renderInstances.size()==assembly.elements.size();return out;
}

} // namespace subspace
