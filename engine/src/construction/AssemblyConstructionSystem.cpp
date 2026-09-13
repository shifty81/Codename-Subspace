#include "construction/AssemblyConstructionSystem.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <queue>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace subspace {
namespace {
double Snap(double value, double quantum) {
    if (quantum <= 0.0) return value;
    return std::round(value / quantum) * quantum;
}

std::string FaceName(AttachmentFace face) {
    switch (face) {
        case AttachmentFace::Fore: return "fore";
        case AttachmentFace::Aft: return "aft";
        case AttachmentFace::Port: return "port";
        case AttachmentFace::Starboard: return "starboard";
        case AttachmentFace::Dorsal: return "dorsal";
        case AttachmentFace::Ventral: return "ventral";
        default: return "none";
    }
}

std::uint64_t Fnv1a(std::uint64_t h, const std::string& value) {
    for (unsigned char c : value) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

std::string Hex64(std::uint64_t value) {
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << value;
    return out.str();
}
}

bool AssemblyConstructionSystem::Begin(AssemblyDefinition definition, std::string* error) {
    if (definition.id.empty() || !definition.assemblyId.IsValid()) {
        if (error) *error = "assembly requires stable id and non-empty definition id";
        return false;
    }
    if (definition.quantumMeters <= 0.0) definition.quantumMeters = DefaultQuantumMeters;
    if (definition.rootSubAssemblyId.empty()) definition.rootSubAssemblyId = "root";
    if (definition.subAssemblies.empty()) definition.subAssemblies.push_back({definition.rootSubAssemblyId, {}});
    if (std::none_of(definition.subAssemblies.begin(), definition.subAssemblies.end(), [&](const auto& s) {
            return s.id == definition.rootSubAssemblyId;
        })) {
        definition.subAssemblies.push_back({definition.rootSubAssemblyId, {}});
    }
    current_ = std::move(definition);
    undo_.clear();
    redo_.clear();
    begun_ = true;
    const auto validation = Validate();
    if (!validation.errors.empty()) {
        if (error) *error = validation.errors.front();
        begun_ = false;
        return false;
    }
    return true;
}

BuildElement* AssemblyConstructionSystem::FindElement(const std::string& id) {
    for (auto& element : current_.elements) if (element.id == id) return &element;
    return nullptr;
}

const BuildElement* AssemblyConstructionSystem::FindElement(const std::string& id) const {
    for (const auto& element : current_.elements) if (element.id == id) return &element;
    return nullptr;
}

const SemanticSocket* AssemblyConstructionSystem::FindSocket(const BuildElement& element, const std::string& id) const {
    for (const auto& socket : element.sockets) if (socket.id == id) return &socket;
    return nullptr;
}

bool AssemblyConstructionSystem::HasSubAssembly(const std::string& id) const {
    return std::any_of(current_.subAssemblies.begin(), current_.subAssemblies.end(), [&](const auto& s) { return s.id == id; });
}

BuildTransform AssemblyConstructionSystem::Quantize(BuildTransform transform, double quantumMeters) {
    transform.position.x = Snap(transform.position.x, quantumMeters);
    transform.position.y = Snap(transform.position.y, quantumMeters);
    transform.position.z = Snap(transform.position.z, quantumMeters);
    return transform;
}

bool AssemblyConstructionSystem::IsQuantized(const Double3& point, double quantumMeters, double epsilon) {
    if (quantumMeters <= 0.0) return false;
    return std::fabs(point.x - Snap(point.x, quantumMeters)) <= epsilon &&
           std::fabs(point.y - Snap(point.y, quantumMeters)) <= epsilon &&
           std::fabs(point.z - Snap(point.z, quantumMeters)) <= epsilon;
}

bool AssemblyConstructionSystem::SocketTypesCompatible(const SemanticSocket& a, const SemanticSocket& b) {
    if (a.id.empty() || b.id.empty()) return false;
    if (a.type.empty() || b.type.empty()) return false;
    return a.type == "universal" || b.type == "universal" || a.type == b.type;
}

bool AssemblyConstructionSystem::ApplyInternal(BuildCommand& command, BuildCommand* inverse, std::string* error) {
    if (!begun_) {
        if (error) *error = "no active assembly edit session";
        return false;
    }

    switch (command.kind) {
        case BuildCommandKind::AddElement: {
            if (command.element.id.empty() || FindElement(command.element.id)) {
                if (error) *error = "add requires a unique element id";
                return false;
            }
            if (command.element.subAssemblyId.empty()) command.element.subAssemblyId = current_.rootSubAssemblyId;
            if (!HasSubAssembly(command.element.subAssemblyId)) {
                if (error) *error = "element references unknown subassembly";
                return false;
            }
            command.element.transform = Quantize(command.element.transform, current_.quantumMeters);
            current_.elements.push_back(command.element);
            if (inverse) {
                inverse->kind = BuildCommandKind::RemoveElement;
                inverse->elementId = command.element.id;
            }
            break;
        }
        case BuildCommandKind::RemoveElement: {
            auto* element = FindElement(command.elementId);
            if (!element) {
                if (error) *error = "remove references unknown element";
                return false;
            }
            command.element = *element;
            command.removedAttachments.clear();
            for (const auto& attachment : current_.attachments) {
                if (attachment.aElementId == command.elementId || attachment.bElementId == command.elementId) {
                    command.removedAttachments.push_back(attachment);
                }
            }
            current_.attachments.erase(std::remove_if(current_.attachments.begin(), current_.attachments.end(), [&](const auto& a) {
                return a.aElementId == command.elementId || a.bElementId == command.elementId;
            }), current_.attachments.end());
            current_.elements.erase(std::remove_if(current_.elements.begin(), current_.elements.end(), [&](const auto& e) {
                return e.id == command.elementId;
            }), current_.elements.end());
            if (inverse) {
                inverse->kind = BuildCommandKind::AddElement;
                inverse->element = command.element;
                inverse->removedAttachments = command.removedAttachments;
            }
            break;
        }
        case BuildCommandKind::MoveElement: {
            auto* element = FindElement(command.elementId);
            if (!element) {
                if (error) *error = "move references unknown element";
                return false;
            }
            command.beforeTransform = element->transform;
            command.afterTransform = Quantize(command.afterTransform, current_.quantumMeters);
            element->transform = command.afterTransform;
            if (inverse) {
                inverse->kind = BuildCommandKind::MoveElement;
                inverse->elementId = command.elementId;
                inverse->afterTransform = command.beforeTransform;
            }
            break;
        }
        case BuildCommandKind::Attach: {
            const auto* a = FindElement(command.attachment.aElementId);
            const auto* b = FindElement(command.attachment.bElementId);
            if (!a || !b || a->id == b->id) {
                if (error) *error = "attachment endpoints are invalid";
                return false;
            }
            const auto* aSocket = FindSocket(*a, command.attachment.aSocketId);
            const auto* bSocket = FindSocket(*b, command.attachment.bSocketId);
            if (!aSocket || !bSocket || !SocketTypesCompatible(*aSocket, *bSocket)) {
                if (error) *error = "attachment sockets are missing or incompatible";
                return false;
            }
            if (std::find(current_.attachments.begin(), current_.attachments.end(), command.attachment) != current_.attachments.end()) {
                if (error) *error = "attachment already exists";
                return false;
            }
            current_.attachments.push_back(command.attachment);
            if (inverse) {
                inverse->kind = BuildCommandKind::Detach;
                inverse->attachment = command.attachment;
            }
            break;
        }
        case BuildCommandKind::Detach: {
            const auto it = std::find(current_.attachments.begin(), current_.attachments.end(), command.attachment);
            if (it == current_.attachments.end()) {
                if (error) *error = "attachment does not exist";
                return false;
            }
            const auto prior = *it;
            current_.attachments.erase(it);
            if (inverse) {
                inverse->kind = BuildCommandKind::Attach;
                inverse->attachment = prior;
            }
            break;
        }
    }

    // Undoing a removal needs to restore the structural edges that were removed
    // with that element. AddElement carries those edges in removedAttachments.
    if (command.kind == BuildCommandKind::AddElement && !command.removedAttachments.empty()) {
        for (const auto& attachment : command.removedAttachments) {
            if (std::find(current_.attachments.begin(), current_.attachments.end(), attachment) == current_.attachments.end()) {
                current_.attachments.push_back(attachment);
            }
        }
    }

    ++current_.revision;
    return true;
}

bool AssemblyConstructionSystem::Apply(BuildCommand command, std::string* error) {
    BuildCommand inverse;
    if (!ApplyInternal(command, &inverse, error)) return false;
    undo_.push_back(std::move(inverse));
    redo_.clear();
    return true;
}

bool AssemblyConstructionSystem::Undo(std::string* error) {
    if (undo_.empty()) {
        if (error) *error = "undo stack is empty";
        return false;
    }
    BuildCommand command = undo_.back();
    undo_.pop_back();
    BuildCommand inverse;
    if (!ApplyInternal(command, &inverse, error)) return false;
    redo_.push_back(std::move(inverse));
    return true;
}

bool AssemblyConstructionSystem::Redo(std::string* error) {
    if (redo_.empty()) {
        if (error) *error = "redo stack is empty";
        return false;
    }
    BuildCommand command = redo_.back();
    redo_.pop_back();
    BuildCommand inverse;
    if (!ApplyInternal(command, &inverse, error)) return false;
    undo_.push_back(std::move(inverse));
    return true;
}

AssemblyValidationResult AssemblyConstructionSystem::Validate(const std::vector<std::string>& requiredCapabilities) const {
    AssemblyValidationResult result;
    if (!begun_) {
        result.errors.push_back("assembly edit session is not active");
        return result;
    }
    if (current_.quantumMeters <= 0.0) result.errors.push_back("construction quantum must be positive");

    std::unordered_set<std::string> subAssemblyIds;
    for (const auto& sub : current_.subAssemblies) {
        if (sub.id.empty() || !subAssemblyIds.insert(sub.id).second) result.errors.push_back("empty or duplicate subassembly id");
    }
    if (!HasSubAssembly(current_.rootSubAssemblyId)) result.errors.push_back("root subassembly is missing");
    for (const auto& sub : current_.subAssemblies) {
        if (!sub.parentId.empty() && !HasSubAssembly(sub.parentId)) result.errors.push_back("subassembly parent is missing: " + sub.id);
    }

    std::unordered_set<std::string> elementIds;
    std::set<std::string> capabilities;
    for (const auto& element : current_.elements) {
        if (element.id.empty() || !elementIds.insert(element.id).second) result.errors.push_back("empty or duplicate element id");
        if (!HasSubAssembly(element.subAssemblyId)) result.errors.push_back("element has invalid subassembly: " + element.id);
        if (!IsQuantized(element.transform.position, current_.quantumMeters)) result.errors.push_back("element translation is off construction quantum: " + element.id);
        if (element.massKg < 0.0) result.errors.push_back("element mass is negative: " + element.id);
        result.totalMassKg += std::max(0.0, element.massKg);
        for (const auto& capability : element.capabilityTags) if (!capability.empty()) capabilities.insert(capability);
        std::unordered_set<std::string> socketIds;
        for (const auto& socket : element.sockets) {
            if (socket.id.empty() || !socketIds.insert(socket.id).second) result.errors.push_back("element has duplicate/empty socket: " + element.id);
        }
    }

    std::unordered_map<std::string, std::vector<std::string>> adjacency;
    for (const auto& attachment : current_.attachments) {
        const auto* a = FindElement(attachment.aElementId);
        const auto* b = FindElement(attachment.bElementId);
        if (!a || !b || a->id == b->id) {
            result.errors.push_back("attachment references invalid element endpoint");
            continue;
        }
        const auto* aSocket = FindSocket(*a, attachment.aSocketId);
        const auto* bSocket = FindSocket(*b, attachment.bSocketId);
        if (!aSocket || !bSocket || !SocketTypesCompatible(*aSocket, *bSocket)) {
            result.errors.push_back("attachment references incompatible semantic sockets");
            continue;
        }
        if (attachment.structural) {
            adjacency[a->id].push_back(b->id);
            adjacency[b->id].push_back(a->id);
        }
    }

    if (current_.elements.size() > 1 && result.errors.empty()) {
        std::unordered_set<std::string> visited;
        std::queue<std::string> queue;
        queue.push(current_.elements.front().id);
        visited.insert(current_.elements.front().id);
        while (!queue.empty()) {
            auto id = queue.front();
            queue.pop();
            for (const auto& neighbor : adjacency[id]) if (visited.insert(neighbor).second) queue.push(neighbor);
        }
        if (visited.size() != current_.elements.size()) result.errors.push_back("structural attachment graph is disconnected");
    }

    for (const auto& required : requiredCapabilities) {
        if (!required.empty() && capabilities.find(required) == capabilities.end()) {
            result.errors.push_back("missing required capability: " + required);
        }
    }
    result.capabilities.assign(capabilities.begin(), capabilities.end());
    if (current_.elements.empty()) result.warnings.push_back("assembly contains no build elements");
    result.valid = result.errors.empty();
    return result;
}

std::string AssemblyConstructionSystem::Fingerprint(const AssemblyDefinition& assembly,
                                                     const AssemblyValidationResult& validation) {
    std::vector<const BuildElement*> elements;
    for (const auto& element : assembly.elements) elements.push_back(&element);
    std::sort(elements.begin(), elements.end(), [](const auto* a, const auto* b) { return a->id < b->id; });
    std::vector<BuildAttachment> attachments = assembly.attachments;
    std::sort(attachments.begin(), attachments.end(), [](const auto& a, const auto& b) {
        return std::tie(a.aElementId, a.aSocketId, a.bElementId, a.bSocketId, a.structural) <
               std::tie(b.aElementId, b.aSocketId, b.bElementId, b.bSocketId, b.structural);
    });

    std::uint64_t hash = 1469598103934665603ull;
    hash = Fnv1a(hash, assembly.id);
    hash = Fnv1a(hash, StableIdentitySystem::ToString(assembly.assemblyId));
    hash = Fnv1a(hash, std::to_string(assembly.quantumMeters));
    for (const auto* element : elements) {
        std::ostringstream line;
        line << element->id << '|' << static_cast<unsigned>(element->kind) << '|' << element->definitionId << '|'
             << element->subAssemblyId << '|' << element->transform.position.x << ',' << element->transform.position.y << ',' << element->transform.position.z
             << '|' << element->massKg;
        hash = Fnv1a(hash, line.str());
        std::vector<std::string> caps = element->capabilityTags;
        std::sort(caps.begin(), caps.end());
        for (const auto& cap : caps) hash = Fnv1a(hash, "cap:" + cap);
        std::vector<SemanticSocket> sockets = element->sockets;
        std::sort(sockets.begin(), sockets.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
        for (const auto& socket : sockets) {
            hash = Fnv1a(hash, "socket:" + socket.id + ":" + socket.type + ":" + FaceName(socket.face));
        }
    }
    for (const auto& attachment : attachments) {
        hash = Fnv1a(hash, attachment.aElementId + ":" + attachment.aSocketId + "->" +
                            attachment.bElementId + ":" + attachment.bSocketId +
                            (attachment.structural ? ":S" : ":N"));
    }
    for (const auto& cap : validation.capabilities) hash = Fnv1a(hash, "compiled-cap:" + cap);
    return "assembly-v1-" + Hex64(hash);
}

AssemblyCompileSnapshot AssemblyConstructionSystem::Compile(const std::vector<std::string>& requiredCapabilities) const {
    const auto validation = Validate(requiredCapabilities);
    AssemblyCompileSnapshot snapshot;
    snapshot.valid = validation.valid;
    snapshot.revision = current_.revision;
    snapshot.elementCount = current_.elements.size();
    snapshot.attachmentCount = current_.attachments.size();
    snapshot.totalMassKg = validation.totalMassKg;
    snapshot.capabilities = validation.capabilities;
    if (snapshot.valid) snapshot.fingerprint = Fingerprint(current_, validation);
    return snapshot;
}

} // namespace subspace
