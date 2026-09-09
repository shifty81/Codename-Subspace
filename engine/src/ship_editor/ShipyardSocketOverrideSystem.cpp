#include "ship_editor/ShipyardSocketOverrideSystem.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace subspace {
namespace {

constexpr const char* kHeader="SUBSPACE_SHIPYARD_SOCKET_OVERRIDES_V1";

bool Near(float a,float b,float epsilon){return std::fabs(a-b)<=epsilon;}

bool SocketEqual(const ShipyardAssemblySocket& a,const ShipyardAssemblySocket& b,float epsilon){
    return a.name==b.name && a.type==b.type &&
        Near(a.x,b.x,epsilon)&&Near(a.y,b.y,epsilon)&&Near(a.z,b.z,epsilon)&&
        Near(a.dirX,b.dirX,epsilon)&&Near(a.dirY,b.dirY,epsilon)&&Near(a.dirZ,b.dirZ,epsilon)&&
        Near(a.insertionDepth,b.insertionDepth,epsilon)&&
        Near(a.upX,b.upX,epsilon)&&Near(a.upY,b.upY,epsilon)&&Near(a.upZ,b.upZ,epsilon);
}

} // namespace

bool ShipyardSocketOverrideSystem::SocketListsEqual(const std::vector<ShipyardAssemblySocket>& a,
                                                     const std::vector<ShipyardAssemblySocket>& b,
                                                     float epsilon){
    if(a.size()!=b.size())return false;
    for(std::size_t i=0;i<a.size();++i)if(!SocketEqual(a[i],b[i],epsilon))return false;
    return true;
}

bool ShipyardSocketOverrideSystem::Save(const std::vector<ShipyardModuleRecord>& baseline,
                                        const std::vector<ShipyardModuleRecord>& edited,
                                        const std::string& path,
                                        std::string* error,
                                        std::size_t* changedModules){
    std::unordered_map<std::string,const ShipyardModuleRecord*> base;
    for(const auto& record:baseline)base[record.source.moduleId]=&record;

    std::ofstream out(path,std::ios::trunc);
    if(!out){if(error)*error="could not open socket override path for writing";return false;}
    out<<kHeader<<"\n";
    out<<std::setprecision(9);
    std::size_t changed=0;
    for(const auto& record:edited){
        const auto it=base.find(record.source.moduleId);
        if(it!=base.end()&&SocketListsEqual(it->second->sockets,record.sockets))continue;
        ++changed;
        out<<"MODULE "<<std::quoted(record.source.moduleId)<<" "<<record.sockets.size()<<"\n";
        for(const auto& socket:record.sockets){
            out<<"SOCKET "<<std::quoted(socket.name)<<" "<<std::quoted(socket.type)<<" "
               <<socket.x<<" "<<socket.y<<" "<<socket.z<<" "
               <<socket.dirX<<" "<<socket.dirY<<" "<<socket.dirZ<<" "
               <<socket.insertionDepth<<" "<<socket.upX<<" "<<socket.upY<<" "<<socket.upZ<<"\n";
        }
        out<<"END\n";
    }
    if(!out.good()){if(error)*error="failed while writing socket overrides";return false;}
    if(changedModules)*changedModules=changed;
    return true;
}

bool ShipyardSocketOverrideSystem::LoadAndApply(std::vector<ShipyardModuleRecord>& catalog,
                                                const std::string& path,
                                                std::string* error,
                                                std::size_t* appliedModules){
    std::ifstream in(path);
    if(!in){if(error)*error="socket override file not found";return false;}
    std::string header;std::getline(in,header);
    if(header!=kHeader){if(error)*error="unsupported socket override document";return false;}

    std::unordered_map<std::string,ShipyardModuleRecord*> byId;
    for(auto& record:catalog)byId[record.source.moduleId]=&record;

    std::size_t applied=0;
    std::string token;
    while(in>>token){
        if(token!="MODULE"){if(error)*error="malformed socket override document: expected MODULE";return false;}
        std::string moduleId;std::size_t socketCount=0;
        if(!(in>>std::quoted(moduleId)>>socketCount)){if(error)*error="malformed MODULE record";return false;}
        std::vector<ShipyardAssemblySocket> sockets;sockets.reserve(socketCount);
        for(std::size_t i=0;i<socketCount;++i){
            std::string socketToken;if(!(in>>socketToken)||socketToken!="SOCKET"){if(error)*error="malformed socket override document: expected SOCKET";return false;}
            ShipyardAssemblySocket socket;
            if(!(in>>std::quoted(socket.name)>>std::quoted(socket.type)
                 >>socket.x>>socket.y>>socket.z
                 >>socket.dirX>>socket.dirY>>socket.dirZ
                 >>socket.insertionDepth>>socket.upX>>socket.upY>>socket.upZ)){
                if(error)*error="malformed SOCKET record";return false;
            }
            socket.manualOverride=true;
            sockets.push_back(std::move(socket));
        }
        std::string end;if(!(in>>end)||end!="END"){if(error)*error="malformed socket override document: expected END";return false;}
        const auto it=byId.find(moduleId);
        if(it!=byId.end()){it->second->sockets=std::move(sockets);++applied;}
    }
    if(appliedModules)*appliedModules=applied;
    return true;
}

} // namespace subspace
