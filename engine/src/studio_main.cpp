// STUDIO-S01: dedicated native Studio entry point. The editor/renderer host
// remains shared with the game until application ownership is extracted.
// Do not start a gameplay session, restore a live refit ship, or duplicate
// ship/asset/blueprint authorities here.
#include "application/NativeGameApplication.h"
#include "platform/NativeWindow.h"

#include <iostream>
#include <string>

namespace {
void PrintUsage() {
    std::cout
        << "Subspace Studio - standalone ship authoring (Studio S01)\n"
        << "Usage: subspace_studio [--studio-smoke] [--help]\n"
        << "  --studio-smoke  Run the native Shipyard smoke path for 8 frames.\n"
        << "  --help          Print this help without initializing graphics.\n"
        << "Shipyard opens an empty authoring document, not a live game refit.\n";
}
}

int main(int argc, char* argv[]) {
    // Help must work on headless machines and before GPU/window init.
    for (int i=1; i<argc; ++i) {
        const std::string arg=argv[i] ? argv[i] : "";
        if (arg=="--help" || arg=="-h") { PrintUsage(); return 0; }
    }

    subspace::NativeGameRunOptions options{};
    options.startShipyard=true; // The dedicated executable has no gameplay mode.
    options.maxFrames=0;
    for (int i=1; i<argc; ++i) {
        const std::string arg=argv[i] ? argv[i] : "";
        if (arg=="--studio-smoke" || arg=="--shipyard-smoke") {
            options.shipyardSmoke=true;
            options.maxFrames=8;
        } else {
            std::cerr << "Unknown Studio option: " << arg << "\n";
            PrintUsage();
            return 64;
        }
    }
    if (!subspace::NativeWindow::IsPlatformBackendAvailable()) {
        std::cerr << "No native window backend is available for Studio on this platform.\n";
        return 2;
    }
    std::cout << "Subspace Studio: native ship authoring host starting...\n";
    subspace::NativeGameApplication app;
    return app.Run(options);
}
