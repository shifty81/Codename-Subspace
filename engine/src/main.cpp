#include <iostream>
#include <string>

#include "application/NativeGameApplication.h"
#include "platform/NativeWindow.h"

int main(int argc, char* argv[])
{
    std::cout << "Codename: Subspace native C++ runtime starting..." << std::endl;

    if (!subspace::NativeWindow::IsPlatformBackendAvailable()) {
        std::cerr << "No native window backend is available for this platform build." << std::endl;
        return 2;
    }

    subspace::NativeGameRunOptions options;
    for (int i=1;i<argc;++i) {
        const std::string arg=argv[i] ? argv[i] : "";
        if (arg=="--runtime-smoke") { options.runtimeSmoke=true; options.maxFrames=4; }
        else if (arg=="--loop") { options.runtimeSmoke=false; options.maxFrames=0; }
        else if (arg=="--shipyard") { options.runtimeSmoke=false; options.startShipyard=true; options.maxFrames=0; }
    }

    subspace::NativeGameApplication app;
    return app.Run(options);
}
