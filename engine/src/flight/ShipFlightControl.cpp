#include "flight/ShipFlightControl.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {

static float Clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

ShipFlightControlOutput EvaluateShipFlightControl(const ShipFlightInputFrame& input,
                                                   const ShipFlightControlProfile& profile) {
    ShipFlightControlOutput output;
    const float precision = input.precisionMode ? profile.precisionMultiplier : 1.0f;
    const float thrust = Clamp01(input.thrust);
    const float reverse = Clamp01(input.reverse);
    const float strafe = std::max(-1.0f, std::min(1.0f, input.strafe));
    const float turn = std::max(-1.0f, std::min(1.0f, input.turn));

    if (input.mainThrustersEnabled) {
        output.forwardForce += thrust * profile.mainThrust * precision;
        output.forwardForce -= reverse * profile.reverseThrust * precision;
        output.mainBurning = thrust > 0.01f;
        output.retroBurning = reverse > 0.01f;
        output.fuelBurnPerSecond += (thrust + reverse * 0.75f) * profile.fuelBurnMain * precision;
    }

    if (input.rcsThrustersEnabled) {
        output.lateralForce = strafe * profile.strafeThrust * precision;
        output.angularForce = turn * profile.turnRate * precision;
        output.leftRcsBurning = turn > 0.01f;
        output.rightRcsBurning = turn < -0.01f;
        output.strafeRcsBurning = std::fabs(strafe) > 0.01f;
        output.fuelBurnPerSecond += (std::fabs(strafe) + std::fabs(turn) * 0.5f) * profile.fuelBurnRcs * precision;
    }

    if (!input.mainThrustersEnabled && !input.rcsThrustersEnabled) {
        output.modeLabel = "full cutoff / ballistic coast";
    } else if (!input.mainThrustersEnabled) {
        output.modeLabel = "main cutoff / RCS only";
    } else if (input.precisionMode) {
        output.modeLabel = "precision burn";
    } else {
        output.modeLabel = "normal burn";
    }

    return output;
}

std::string ShipFlightControlSummary(const ShipFlightControlOutput& output) {
    std::ostringstream ss;
    ss << output.modeLabel
       << " fuel=" << output.fuelBurnPerSecond
       << " fwd=" << output.forwardForce
       << " lat=" << output.lateralForce
       << " turn=" << output.angularForce;
    return ss.str();
}

} // namespace subspace
