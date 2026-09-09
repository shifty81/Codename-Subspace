#include "runtime/SandboxVerticalSliceSystem.h"
#include <algorithm>
namespace subspace {
std::vector<VerticalSliceCheckpoint> SandboxVerticalSliceSystem::Pass220Checklist(bool a,bool b,bool c,bool d,bool e,bool f,bool g,bool h,bool i,bool j)const{return{{"main_menu",a},{"starter_ship",b},{"hangar",c},{"undock",d},{"scan_site",e},{"gather",f},{"field_fabrication",g},{"market_services",h},{"hire_captain",i},{"save_continue",j}};}
bool SandboxVerticalSliceSystem::Complete(const std::vector<VerticalSliceCheckpoint>&c)const{return !c.empty()&&std::all_of(c.begin(),c.end(),[](const auto&x){return x.satisfied;});}
} // namespace subspace
