#pragma once

#include <string>

namespace subspace {

struct ShipFlightInputFrame {
    float thrust = 0.0f;
    float reverse = 0.0f;
    float strafe = 0.0f;
    float turn = 0.0f;
    bool mainThrustersEnabled = true;
    bool rcsThrustersEnabled = true;
    bool precisionMode = false;
};

struct ShipFlightControlProfile {
    float mainThrust = 430.0f;
    float reverseThrust = 280.0f;
    float strafeThrust = 320.0f;
    float turnRate = 2.7f;
    float maxSpeed = 620.0f;
    float fuelBurnMain = 1.0f;
    float fuelBurnRcs = 0.24f;
    float precisionMultiplier = 0.45f;
};

struct ShipFlightControlOutput {
    float forwardForce = 0.0f;
    float lateralForce = 0.0f;
    float angularForce = 0.0f;
    float fuelBurnPerSecond = 0.0f;
    bool mainBurning = false;
    bool retroBurning = false;
    bool leftRcsBurning = false;
    bool rightRcsBurning = false;
    bool strafeRcsBurning = false;
    std::string modeLabel;
};

ShipFlightControlOutput EvaluateShipFlightControl(const ShipFlightInputFrame& input,
                                                   const ShipFlightControlProfile& profile);
std::string ShipFlightControlSummary(const ShipFlightControlOutput& output);

} // namespace subspace
