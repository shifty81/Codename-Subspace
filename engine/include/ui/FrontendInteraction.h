#pragma once

#include "ui/FrontendFlowSystem.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace subspace {

/// Semantic frontend commands shared by the native renderer and input router.
/// Keeping the visible control rectangles and hit testing in one contract
/// prevents the main menu from becoming painted text with unrelated click logic.
enum class FrontendCommand {
    None = 0,
    MainNewSandbox,
    MainShipyard,
    MainLoadSandbox,
    MainSettings,
    MainCredits,
    MainExit,
    NewToggleStory,
    NewToggleCoop,
    NewSeedPrevious,
    NewSeedNext,
    NewContinue,
    Back,
    StarterPrevious,
    StarterNext,
    StarterAccept,
    HangarUndock
};

struct FrontendRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    bool Contains(float px, float py) const {
        return px >= x && py >= y && px <= x + width && py <= y + height;
    }
};

struct FrontendControl {
    FrontendCommand command = FrontendCommand::None;
    FrontendRect bounds{};
    const char* label = "";
    bool primary = false;
};

inline std::vector<FrontendControl> BuildFrontendControls(FrontendScreen screen, int viewportWidth, int viewportHeight)
{
    const float w = static_cast<float>(std::max(1, viewportWidth));
    const float h = static_cast<float>(std::max(1, viewportHeight));
    const float left = w * 0.5f - 220.0f;
    const float fullWidth = 440.0f;
    const float rowHeight = 38.0f;
    const float gap = 10.0f;
    std::vector<FrontendControl> controls;

    const auto add = [&](FrontendCommand command, float x, float y, float width, const char* label, bool primary = false) {
        controls.push_back({command, {x, y, width, rowHeight}, label, primary});
    };

    switch (screen) {
        case FrontendScreen::MainMenu: {
            float y = h * 0.40f;
            add(FrontendCommand::MainNewSandbox, left, y, fullWidth, "NEW SANDBOX", true); y += rowHeight + gap;
            add(FrontendCommand::MainShipyard, left, y, fullWidth, "SHIPYARD / MODEL + PCG STUDIO"); y += rowHeight + gap;
            add(FrontendCommand::MainLoadSandbox, left, y, fullWidth, "LOAD SANDBOX"); y += rowHeight + gap;
            add(FrontendCommand::MainSettings, left, y, fullWidth, "SETTINGS"); y += rowHeight + gap;
            add(FrontendCommand::MainCredits, left, y, fullWidth, "CREDITS"); y += rowHeight + gap;
            add(FrontendCommand::MainExit, left, y, fullWidth, "EXIT");
            break;
        }
        case FrontendScreen::NewSandbox: {
            const float y = h * 0.47f;
            add(FrontendCommand::NewToggleStory, left, y, fullWidth, "STORY SPINE");
            add(FrontendCommand::NewToggleCoop, left, y + 48.0f, fullWidth, "CO-OP");
            add(FrontendCommand::NewSeedPrevious, left, y + 96.0f, 84.0f, "SEED -");
            add(FrontendCommand::NewSeedNext, left + fullWidth - 84.0f, y + 96.0f, 84.0f, "SEED +");
            add(FrontendCommand::NewContinue, left, y + 154.0f, fullWidth, "CONTINUE TO STARTER SHIP", true);
            add(FrontendCommand::Back, left, y + 202.0f, fullWidth, "BACK");
            break;
        }
        case FrontendScreen::StartingShip: {
            const float y = h * 0.50f;
            add(FrontendCommand::StarterPrevious, left, y, 120.0f, "PREVIOUS");
            add(FrontendCommand::StarterNext, left + fullWidth - 120.0f, y, 120.0f, "NEXT");
            add(FrontendCommand::StarterAccept, left, y + 64.0f, fullWidth, "ACCEPT STARTER SHIP", true);
            add(FrontendCommand::Back, left, y + 112.0f, fullWidth, "BACK");
            break;
        }
        case FrontendScreen::StationHangar: {
            const float y = h * 0.52f;
            add(FrontendCommand::HangarUndock, left, y, fullWidth, "UNDOCK / START SANDBOX", true);
            add(FrontendCommand::Back, left, y + 48.0f, fullWidth, "BACK");
            break;
        }
        case FrontendScreen::LoadSandbox:
        case FrontendScreen::Settings:
        case FrontendScreen::Credits: {
            add(FrontendCommand::Back, left, h * 0.62f, fullWidth, "BACK", true);
            break;
        }
        default:
            break;
    }
    return controls;
}

inline FrontendCommand HitTestFrontendControls(const std::vector<FrontendControl>& controls, float x, float y)
{
    for (const auto& control : controls) {
        if (control.bounds.Contains(x, y)) return control.command;
    }
    return FrontendCommand::None;
}

inline FrontendCommand DefaultFrontendCommand(FrontendScreen screen)
{
    switch (screen) {
        case FrontendScreen::MainMenu: return FrontendCommand::MainNewSandbox;
        case FrontendScreen::NewSandbox: return FrontendCommand::NewContinue;
        case FrontendScreen::StartingShip: return FrontendCommand::StarterAccept;
        case FrontendScreen::StationHangar: return FrontendCommand::HangarUndock;
        case FrontendScreen::LoadSandbox:
        case FrontendScreen::Settings:
        case FrontendScreen::Credits: return FrontendCommand::Back;
        default: return FrontendCommand::None;
    }
}

inline FrontendCommand StepFrontendCommand(const std::vector<FrontendControl>& controls, FrontendCommand current, int direction)
{
    if (controls.empty()) return FrontendCommand::None;
    std::size_t index = 0;
    const auto it = std::find_if(controls.begin(), controls.end(), [&](const FrontendControl& control) { return control.command == current; });
    if (it != controls.end()) index = static_cast<std::size_t>(std::distance(controls.begin(), it));
    if (direction >= 0) index = (index + 1u) % controls.size();
    else index = (index + controls.size() - 1u) % controls.size();
    return controls[index].command;
}

} // namespace subspace
