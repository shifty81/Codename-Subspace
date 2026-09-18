#include "studio/StudioDocumentShortcuts.h"
#include <iostream>
using namespace subspace;
int main(){
    unsigned assertions=0,failed=0;
    auto check=[&](bool condition,const char* label){++assertions;if(!condition){++failed;std::cerr<<"FAIL "<<label<<'\n';}};
    using A=StudioDocumentShortcut;
    check(StudioDocumentShortcuts::Resolve(true,false,true,false,false)==A::Open,"Ctrl+O opens document");
    check(StudioDocumentShortcuts::Resolve(true,false,false,true,false)==A::New,"Ctrl+N creates document");
    check(StudioDocumentShortcuts::Resolve(true,false,false,false,true)==A::Save,"Ctrl+S saves");
    check(StudioDocumentShortcuts::Resolve(true,true,false,false,true)==A::SaveAs,"Ctrl+Shift+S saves as");
    check(StudioDocumentShortcuts::Resolve(false,false,true,true,true)==A::None,"unmodified keys never launch document commands");
    check(StudioDocumentShortcuts::Resolve(false,true,false,false,true)==A::None,"Shift+S is modeling input");
    check(StudioDocumentShortcuts::Resolve(true,true,true,false,true)==A::Open,"open takes precedence over save");
    check(StudioDocumentShortcuts::Resolve(true,false,false,true,true)==A::New,"new takes precedence over save");
    check(StudioDocumentShortcuts::Resolve(true,false,false,false,false)==A::None,"idle control is inert");
    std::cout<<"Studio document shortcuts: "<<assertions-failed<<"/"<<assertions<<" assertions passed\n";
    return failed?1:0;
}
