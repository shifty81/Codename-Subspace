#pragma once

#include "ship_editor/ShipyardDocumentSystem.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace subspace {

struct ShipyardHistoryEntry {
    std::string label;
    ShipyardDocumentSnapshot before{};
    ShipyardDocumentSnapshot after{};
};

class ShipyardHistorySystem {
public:
    explicit ShipyardHistorySystem(std::size_t capacity = 128) : capacity_(capacity ? capacity : 1) {}

    bool Begin(std::string label, const ShipyardDocument& document);
    bool Commit(const ShipyardDocument& document);
    void Cancel();

    bool CanUndo() const { return !undo_.empty(); }
    bool CanRedo() const { return !redo_.empty(); }
    bool Undo(ShipyardDocument& document, std::string* label = nullptr);
    bool Redo(ShipyardDocument& document, std::string* label = nullptr);
    void Clear();

    std::size_t UndoCount() const { return undo_.size(); }
    std::size_t RedoCount() const { return redo_.size(); }
    bool TransactionOpen() const { return pending_.has_value(); }

private:
    struct Pending {
        std::string label;
        ShipyardDocumentSnapshot before{};
        std::uint64_t startingRevision = 0;
    };

    std::size_t capacity_ = 128;
    std::optional<Pending> pending_;
    std::vector<ShipyardHistoryEntry> undo_;
    std::vector<ShipyardHistoryEntry> redo_;
};

} // namespace subspace
