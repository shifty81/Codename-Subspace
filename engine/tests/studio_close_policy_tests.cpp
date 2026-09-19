#include "studio/StudioClosePolicy.h"
#include <iostream>
using namespace subspace;
int main(){
    int checks=0,failed=0;
    auto expect=[&](bool success,const char* name){++checks;if(!success){++failed;std::cerr<<"FAIL "<<name<<'\n';}};
    constexpr StudioCloseState clean{};
    constexpr StudioCloseState assembly{true,false,false,false,false};
    constexpr StudioCloseState model{true,false,false,true,false};
    constexpr StudioCloseState interior{false,false,false,false,true};
    constexpr StudioCloseState overrides{false,true,true,false,false};
    expect(!StudioClosePolicy::NeedsPrompt(clean),"clean window closes without confirmation");
    expect(StudioClosePolicy::NeedsPrompt(assembly),"dirty blueprint requests confirmation");
    expect(StudioClosePolicy::NeedsPrompt(model),"model draft requests confirmation");
    expect(StudioClosePolicy::NeedsPrompt(interior),"interior draft requests confirmation");
    expect(StudioClosePolicy::NeedsPrompt(overrides),"override drafts request confirmation");
    expect(!StudioClosePolicy::NeedsExplicitDataLossWarning(assembly),"blueprint has recovery lane");
    expect(StudioClosePolicy::NeedsExplicitDataLossWarning(model),"model needs explicit warning");
    expect(StudioClosePolicy::NeedsExplicitDataLossWarning(interior),"interior needs warning");
    expect(StudioClosePolicy::NeedsExplicitDataLossWarning(overrides),"overrides need warning");
    expect(StudioClosePolicy::MayCloseAfterSave(clean),"no work lost on clean close");
    expect(!StudioClosePolicy::MayCloseAfterSave(model),"blueprint save cannot certify model");
    expect(!StudioClosePolicy::MayCloseWithRecovery(assembly,false,true),"failed recovery vetoes close");
    expect(StudioClosePolicy::MayCloseWithRecovery(assembly,true,false),"verified blueprint recovery permits close");
    expect(!StudioClosePolicy::MayCloseWithRecovery(model,true,false),"unacknowledged model loss vetoes close");
    expect(StudioClosePolicy::MayCloseWithRecovery(model,true,true),"explicitly acknowledged partial recovery");
    expect(!StudioClosePolicy::MayCloseWithRecovery(interior,true,false),"interior warning cannot be bypassed");
    expect(StudioClosePolicy::MayCloseWithRecovery(interior,false,true),"interior-only explicit discard permitted");
    std::cout<<"Studio close policy: "<<(checks-failed)<<"/"<<checks<<" assertions passed\n";
    return failed?1:0;
}
