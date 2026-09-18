#pragma once
#include <filesystem>
#include <string>

namespace subspace {
// Dialogs belong to the Studio process, never the shared game window loop.
// False + empty error means the user canceled; false + error means failure.
struct StudioFileDialog {
    static bool ChooseOpen(const std::filesystem::path& directory,
                           std::filesystem::path& selected,std::string& error);
    static bool ChooseSaveAs(const std::filesystem::path& directory,
                             const std::filesystem::path& current,
                             std::filesystem::path& selected,std::string& error);
    static void ShowError(const std::string& message);
};
}
