#include "studio/StudioRecoveryPathPolicy.h"
#include "studio/StudioUnsavedWorkPolicy.h"
#include <iostream>
using namespace subspace;
int main(){
    unsigned assertions=0,failed=0;
    auto check=[&](bool condition,const char* label){++assertions;if(!condition){++failed;std::cerr<<"FAIL "<<label<<'\n';}};
    const std::filesystem::path base="test_root/dist/blueprints";
    const auto a=StudioRecoveryPathPolicy::Candidate(base,123,0);
    const auto b=StudioRecoveryPathPolicy::Candidate(base,123,1);
    check(StudioRecoveryPathPolicy::Candidate({},123,0).empty(),"missing authority cannot create recovery path");
    check(a.parent_path()==base/"recovery","recovery is in isolated directory");
    check(a.extension()==".subspace_ship","recovery reopens with canonical extension");
    check(a!=b,"sequence prevents collision");
    check(a!=base/"ship.subspace_ship","does not overwrite the opened document");
    check(StudioRecoveryPathPolicy::NeedsRecovery(true),"dirty work triggers recovery");
    check(!StudioRecoveryPathPolicy::NeedsRecovery(false),"clean work does not create recovery noise");
    check(!StudioUnsavedWorkPolicy::HasUnsaved({}),"clean authoring session");
    check(StudioUnsavedWorkPolicy::HasUnsaved({true,false,false,false,false}),"blueprint edits protected");
    check(StudioUnsavedWorkPolicy::HasUnsaved({false,true,false,false,false}),"socket overrides protected");
    check(StudioUnsavedWorkPolicy::HasUnsaved({false,false,true,false,false}),"definition overrides protected");
    check(StudioUnsavedWorkPolicy::HasUnsaved({false,false,false,true,false}),"model drafts protected");
    check(StudioUnsavedWorkPolicy::HasUnsaved({false,false,false,false,true}),"interior drafts protected");
    check(!StudioUnsavedWorkPolicy::HasUnsupportedRecovery({true,false,false,false,false}),"ordinary blueprint recoverable");
    check(StudioUnsavedWorkPolicy::HasUnsupportedRecovery({false,true,false,false,false}),"socket overrides cannot claim full recovery");
    check(StudioUnsavedWorkPolicy::HasUnsupportedRecovery({false,false,true,false,false}),"definition overrides cannot claim full recovery");
    check(StudioUnsavedWorkPolicy::HasUnsupportedRecovery({false,false,false,true,false}),"model draft cannot claim full recovery");
    check(!StudioUnsavedWorkPolicy::HasUnsupportedRecovery({false,false,false,true,false},true),"verified separate model recovery covers model draft");
    check(StudioUnsavedWorkPolicy::HasUnsupportedRecovery({false,true,false,true,false},true),"model recovery does not excuse unpublished socket overrides");
    check(StudioUnsavedWorkPolicy::HasUnsupportedRecovery({false,false,false,false,true}),"interior draft cannot claim full recovery");
    std::cout<<"Studio recovery policy: "<<assertions-failed<<"/"<<assertions<<" assertions passed\n";
    return failed?1:0;
}
