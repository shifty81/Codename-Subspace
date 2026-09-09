#pragma once
#include <cstdint>
#include <string>
namespace subspace {
struct SaveRecoveryEnvelope { int schemaVersion=1; std::string payload; std::string checksum; std::string recoverySlot; };
class SaveRecoverySystem { public: SaveRecoveryEnvelope Wrap(const std::string&payload,const std::string&slot="autosave") const; bool Validate(const SaveRecoveryEnvelope&e) const; bool CanMigrate(int schemaVersion) const; SaveRecoveryEnvelope Migrate(SaveRecoveryEnvelope e) const; static std::string Checksum(const std::string&payload); };
}
