#pragma once

#include "ships/Ship.h"

#include <string>
#include <vector>

namespace subspace {

// Outcome of a ship validation check.
struct ValidationResult {
    bool valid = true;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    void AddError(const std::string& msg);
    void AddWarning(const std::string& msg);
};

// Validates ship designs inside the editor before they are saved or used.
class ShipValidator {
public:
    // Run all validation checks.
    static ValidationResult Validate(const Ship& ship);

    // Individual checks (can be called independently).
    static bool HasBlocks(const Ship& ship);
    static bool IsConnected(const Ship& ship);
    static bool HasEngine(const Ship& ship);
    static bool HasGenerator(const Ship& ship);
    static bool MassWithinLimit(const Ship& ship, float maxMass);
    static bool BlockCountWithinLimit(const Ship& ship, size_t maxBlocks);
};

} // namespace subspace
