#pragma once

namespace subspace {
// Context-owned command routing. The shared NativeWindow reports physical
// actions; only Studio interprets Ctrl+O/N/S as document commands.
enum class StudioDocumentShortcut { None, Open, New, Save, SaveAs };
struct StudioDocumentShortcuts {
    static constexpr StudioDocumentShortcut Resolve(bool control,bool shift,
                                                       bool openPressed,bool newPressed,
                                                       bool savePressed) noexcept {
        if(!control)return StudioDocumentShortcut::None;
        if(openPressed)return StudioDocumentShortcut::Open;
        if(newPressed)return StudioDocumentShortcut::New;
        if(savePressed)return shift?StudioDocumentShortcut::SaveAs:StudioDocumentShortcut::Save;
        return StudioDocumentShortcut::None;
    }
};
}
