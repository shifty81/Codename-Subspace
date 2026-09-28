#include "studio/StudioPlacementWorkflowPolicy.h"
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {int failures=0, assertions=0; void Check(bool ok,const char* n){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<n<<"\n";}else std::cout<<"[PASS] "<<n<<"\n";}}
int main(){
    Check(std::fabs(StudioPlacementWorkflowPolicy::DuplicateOffset(true)-.25f)<.0001f,"snapped duplicate starts one placement step away");
    Check(std::fabs(StudioPlacementWorkflowPolicy::DuplicateOffset(false)-.10f)<.0001f,"free duplicate uses a small visible offset");
    Check(StudioPlacementWorkflowPolicy::SelectionAfterRemoval(1,3)==1,"delete selects the item that shifted into the removed slot");
    Check(StudioPlacementWorkflowPolicy::SelectionAfterRemoval(3,3)==2,"delete of last item selects the new last item");
    Check(StudioPlacementWorkflowPolicy::SelectionAfterRemoval(0,0)==0,"empty selection remains normalized");
    Check(StudioPlacementWorkflowPolicy::CommitIsSingleUndoUnit(),"placement commit is one undo unit");
    Check(StudioPlacementWorkflowPolicy::NormalDeletePreservesDescendants(),"normal delete preserves descendants");
    std::cout<<"R63 placement policy assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
