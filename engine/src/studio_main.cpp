// S02-S10: editor-owned native executable. Never construct NativeGameApplication.
#include "studio/StudioApplication.h"
#include "platform/NativeWindow.h"
#include <filesystem>
#include <iostream>
#include <string>

namespace {
void Usage(){
    std::cout<<"Subspace Studio S02-S10 - independent ship authoring host\n"
             <<"Usage: subspace_studio [--open <blueprint.subspace_ship>] [--studio-smoke] [--help]\n"
             <<"  --open          Open an existing canonical Subspace blueprint.\n"
             <<"  --studio-smoke  Start independent Studio and run eight frames.\n"
             <<"  --help          Show this help without GPU/window initialization.\n"
             <<"ESC cancels editing; Studio has no gameplay main menu.\n";
}
}
int main(int argc,char* argv[]){
    std::filesystem::path open;
    std::uint64_t maxFrames=0;
    for(int i=1;i<argc;++i){
        const std::string arg=argv[i]?argv[i]:"";
        if(arg=="--help"||arg=="-h"){Usage();return 0;}
        if(arg=="--studio-smoke"||arg=="--shipyard-smoke"){maxFrames=8;continue;}
        if(arg=="--open"){
            if(++i>=argc){std::cerr<<"--open requires a blueprint path\n";return 64;}
            open=argv[i];continue;
        }
        std::cerr<<"Unknown Studio option: "<<arg<<'\n';Usage();return 64;
    }
    if(!subspace::NativeWindow::IsPlatformBackendAvailable()){
        std::cerr<<"No native window backend is available for Studio.\n";return 2;
    }
    subspace::StudioApplication studio;
    return studio.Run(open,maxFrames);
}
