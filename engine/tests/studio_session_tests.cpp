#include "studio/StudioSessionPolicy.h"
#include <iostream>
using namespace subspace;
int main(){
    int checks=0,fail=0;
    auto check=[&](bool ok,const char* name){++checks;if(!ok){++fail;std::cerr<<"FAIL "<<name<<'\n';}};
    check(StudioSessionPolicy::Escape({true,true,true,true,true})==StudioEscapeAction::CancelPlacement,"drag wins");
    check(StudioSessionPolicy::Escape({false,true,true,true,true})==StudioEscapeAction::CancelTransform,"transform wins");
    check(StudioSessionPolicy::Escape({false,false,true,true,true})==StudioEscapeAction::CloseCommandPalette,"palette wins");
    check(StudioSessionPolicy::Escape({false,false,false,true,true})==StudioEscapeAction::CloseMenu,"menu wins");
    check(StudioSessionPolicy::Escape({false,false,false,false,true})==StudioEscapeAction::ClearConstraint,"constraint wins");
    check(StudioSessionPolicy::Escape({})==StudioEscapeAction::NoOp,"idle ESC stays in Studio");
    check(StudioSessionPolicy::MayDiscard(false,false),"clean document can new");
    check(!StudioSessionPolicy::MayDiscard(true,false),"dirty document cannot be discarded");
    check(StudioSessionPolicy::MayDiscard(true,true),"explicit discard approval");
    check(StudioSessionPolicy::MayPublish(true,true,true,true,true),"certified ready");
    for(int i=0;i<5;++i){bool v[]={true,true,true,true,true};v[i]=false;check(!StudioSessionPolicy::MayPublish(v[0],v[1],v[2],v[3],v[4]),"each publication gate required");}
    std::cout<<"Studio session: "<<checks-fail<<"/"<<checks<<" assertions passed\n";
    return fail?1:0;
}
