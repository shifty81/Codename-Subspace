#include "content/ShipyardCertificationSystem.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::string input, output;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--input" && i + 1 < argc) input = argv[++i];
        else if (a == "--output" && i + 1 < argc) output = argv[++i];
    }
    if (input.empty() || output.empty()) {
        std::cerr << "Usage: subspace_shipyard_certifier --input <normalized-obj-root> --output <certified-root>\n";
        return 2;
    }
    subspace::ShipyardCorpusCertificationSummary s;
    std::string error;
    if (!subspace::ShipyardCertificationSystem::CertifyCorpus(input, output, &s, &error)) {
        std::cerr << "[FAIL] Shipyard certification: " << error << "\n";
        return 1;
    }
    std::cout << "[PASS] Shipyard Canonical Authority Certification R5\n"
              << "SOURCE_OBJECTS=" << s.sourceObjects << "\nGRADE_A=" << s.gradeA
              << "\nREJECTED=" << s.rejected
              << "\nMULTI_ISLAND_DIAGNOSTICS=" << s.multiIslandSourceObjects << "\n";
    return 0;
}
