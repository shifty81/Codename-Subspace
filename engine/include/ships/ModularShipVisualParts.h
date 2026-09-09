#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ShipVisualPartRole {
    Cockpit,
    Hull,
    Engine,
    RcsThruster,
    Cargo,
    Weapon,
    Shield,
    Scanner,
    Utility
};

struct ShipVisualPartSocket {
    std::string socketId;
    float x = 0.0f;
    float y = 0.0f;
    float rotationDegrees = 0.0f;
};

struct ShipVisualPartDefinition {
    std::string partId;
    ShipVisualPartRole role = ShipVisualPartRole::Utility;
    std::string displayName;
    std::uint32_t tint = 0xAABED0u;
    float mass = 1.0f;
    float powerDraw = 0.0f;
    float thrust = 0.0f;
    float torque = 0.0f;
    float cargo = 0.0f;
    int tier = 1;
    std::vector<ShipVisualPartSocket> sockets;
};

const char* ShipVisualPartRoleName(ShipVisualPartRole role);
std::vector<ShipVisualPartDefinition> CreateStarterVisualShipPartCatalog();
float SumVisualPartThrust(const std::vector<ShipVisualPartDefinition>& parts);
float SumVisualPartCargo(const std::vector<ShipVisualPartDefinition>& parts);

} // namespace subspace
