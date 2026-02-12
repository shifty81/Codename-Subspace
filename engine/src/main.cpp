#include <iostream>
#include "core/Engine.h"

int main(int argc, char* argv[])
{
    std::cout << "Codename: Subspace starting..." << std::endl;

    subspace::Engine engine;
    engine.Initialize();
    engine.Run();
    engine.Shutdown();

    return 0;
}
