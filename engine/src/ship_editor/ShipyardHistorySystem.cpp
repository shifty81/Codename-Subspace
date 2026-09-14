#include "ship_editor/ShipyardHistorySystem.h"

namespace subspace {

bool ShipyardHistorySystem::Begin(std::string label, const ShipyardDocument& document) {
    if (pending_) return false;
    pending_ = Pending{std::move(label), ShipyardDocumentSystem::Capture(document), document.revision};
    return true;
}

bool ShipyardHistorySystem::Commit(const ShipyardDocument& document) {
    if (!pending_) return false;
    if (document.revision == pending_->startingRevision) {
        pending_.reset();
        return false;
    }
    ShipyardHistoryEntry entry{pending_->label, pending_->before, ShipyardDocumentSystem::Capture(document)};
    pending_.reset();
    undo_.push_back(std::move(entry));
    if (undo_.size() > capacity_) undo_.erase(undo_.begin());
    redo_.clear();
    return true;
}

void ShipyardHistorySystem::Cancel() {
    pending_.reset();
}

bool ShipyardHistorySystem::Undo(ShipyardDocument& document, std::string* label) {
    if (pending_ || undo_.empty()) return false;
    auto entry = std::move(undo_.back());
    undo_.pop_back();
    if (label) *label = entry.label;
    ShipyardDocumentSystem::Restore(document, entry.before);
    redo_.push_back(std::move(entry));
    return true;
}

bool ShipyardHistorySystem::Redo(ShipyardDocument& document, std::string* label) {
    if (pending_ || redo_.empty()) return false;
    auto entry = std::move(redo_.back());
    redo_.pop_back();
    if (label) *label = entry.label;
    ShipyardDocumentSystem::Restore(document, entry.after);
    undo_.push_back(std::move(entry));
    return true;
}

void ShipyardHistorySystem::Clear() {
    pending_.reset();
    undo_.clear();
    redo_.clear();
}

} // namespace subspace
