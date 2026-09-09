#pragma once

namespace subspace {

enum class RuntimeMode {
    None,
    MainMenu,
    Loading,
    Playing,
    DevelopmentPlay,
    Authoring,
    Paused,
    ShuttingDown
};

} // namespace subspace
