#include "studio/StudioNativeCloseGuard.h"
#include <iostream>
#include <string>
int main(){
    subspace::StudioNativeCloseGuard guard;
    guard.Detach();guard.Detach();
#ifndef _WIN32
    std::string error;
    bool invoked=false;
    const bool installed=guard.Install("Subspace Studio - Ship Authoring",[&]{invoked=true;return false;},error);
    if(installed||error.empty()||invoked){std::cerr<<"FAIL non-Windows close guard must fail closed\n";return 1;}
#endif
    std::cout<<"Studio native close guard: platform guard and idempotent detach PASS\n";
    return 0;
}
