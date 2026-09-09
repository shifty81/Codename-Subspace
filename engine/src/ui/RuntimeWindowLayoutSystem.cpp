#include "ui/RuntimeWindowLayoutSystem.h"
#include <algorithm>
namespace subspace {
RuntimeWindowLayout RuntimeWindowLayoutSystem::DefaultFlightLayout(int w,int h){RuntimeWindowLayout l;l.windows={{"selected","SELECTED OBJECT",float(w-394),72,360,150,RuntimeWindowDock::Right,RuntimeWindowMode::Normal,true,false,.88f,10},{"contacts","CONTACTS",float(w-430),238,396,float(std::max(260,h-338)),RuntimeWindowDock::Right,RuntimeWindowMode::Normal,true,false,.86f,9},{"comms","COMMS",96,float(h-204),360,170,RuntimeWindowDock::Bottom,RuntimeWindowMode::Compact,false,false,.82f,8},{"drones","DRONES",float(w-430),float(h-250),396,210,RuntimeWindowDock::Right,RuntimeWindowMode::Compact,false,false,.84f,8}};return l;}
bool RuntimeWindowLayoutSystem::Move(RuntimeWindowLayout& l,const std::string&id,float x,float y){for(auto&w:l.windows)if(w.id==id&&!w.locked){w.x=x;w.y=y;w.dock=RuntimeWindowDock::Floating;return true;}return false;}
bool RuntimeWindowLayoutSystem::Resize(RuntimeWindowLayout& l,const std::string&id,float width,float height){for(auto&w:l.windows)if(w.id==id&&!w.locked){w.width=std::max(120.0f,width);w.height=std::max(80.0f,height);return true;}return false;}
bool RuntimeWindowLayoutSystem::SetMode(RuntimeWindowLayout& l,const std::string&id,RuntimeWindowMode mode){for(auto&w:l.windows)if(w.id==id){w.mode=mode;return true;}return false;}
} // namespace subspace
