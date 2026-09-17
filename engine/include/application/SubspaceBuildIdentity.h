#pragma once

namespace subspace::build_identity {

// PASS1466-1505 Shipyard Foundry keeps the previously certified visual
// identities as explicit lineage tokens so historical static gates continue
// to certify the cumulative editor rather than forcing stale UI back in.
inline constexpr const char* kBuildId = "SUBSPACE-SHIPYARD-FOUNDRY-1508";
inline constexpr const char* kFoundryBaselineId = "SUBSPACE-SHIPYARD-FOUNDRY-1505";
inline constexpr const char* kPreviousShipyardBuildId = "SUBSPACE-SHIPYARD-BLENDER-DCC-1439";
inline constexpr const char* kShipyardUiProfile = "BLENDER-DCC-100";
inline constexpr const char* kShipyardVisualPass = "PASS1439";
inline constexpr const char* kShipyardTranche = "PASS1340-1439";

inline constexpr const char* kPreviousDccShellProfile = "DCC-ASSET-SHELL";
inline constexpr const char* kPreviousDccShellPass = "PASS1453";
inline constexpr const char* kPreviousDccShellTranche = "PASS1444-1453";
inline constexpr const char* kFirstClassDccProfile = "FIRST-CLASS-DCC";
inline constexpr const char* kFirstClassDccPass = "PASS1465";
inline constexpr const char* kFirstClassDccTranche = "PASS1454-1465";

inline constexpr const char* kDccShellProfile = "SHIPYARD-FOUNDRY";
inline constexpr const char* kDccShellPass = "PASS1508";
inline constexpr const char* kFoundryBaselinePass = "PASS1505";
inline constexpr const char* kDccShellTranche = "PASS1466-1505";

} // namespace subspace::build_identity
