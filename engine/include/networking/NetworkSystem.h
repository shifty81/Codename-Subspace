#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class MessageType { JoinSector, LeaveSector, EntityUpdate, SectorJoined, ChatMessage };

/// A network message exchanged between client and server.
struct NetworkMessage {
    MessageType type = MessageType::ChatMessage;
    std::string data;
    double timestamp = 0.0; // seconds since epoch

    NetworkMessage();
    NetworkMessage(MessageType t, const std::string& d);

    /// Serialize to bytes (type + length-prefixed data + timestamp).
    std::vector<uint8_t> Serialize() const;

    /// Deserialize from bytes.
    static NetworkMessage Deserialize(const std::vector<uint8_t>& bytes);
};

/// Represents a connected client (transport-agnostic).
class ClientConnection {
public:
    ClientConnection(uint32_t id, const std::string& name = "");

    uint32_t GetId() const;
    const std::string& GetName() const;
    bool IsConnected() const;
    void Disconnect();

    /// Queue a message for sending to this client.
    void QueueMessage(const NetworkMessage& msg);

    /// Retrieve and clear all queued outgoing messages.
    std::vector<NetworkMessage> FlushOutbox();

    /// Push a message into the receive queue.
    void ReceiveMessage(const NetworkMessage& msg);

    /// Retrieve and clear all received messages.
    std::vector<NetworkMessage> FlushInbox();

    /// Get the sector this client is currently in.
    const std::string& GetCurrentSector() const;

    /// Set the sector this client is currently in.
    void SetCurrentSector(const std::string& sector);

private:
    uint32_t id_;
    std::string name_;
    bool connected_;
    std::vector<NetworkMessage> outbox_;
    std::vector<NetworkMessage> inbox_;
    std::string currentSector_;
};

/// Manages clients within a single sector.
class SectorServer {
public:
    SectorServer(const std::string& id);

    const std::string& GetId() const;
    size_t GetClientCount() const;

    void AddClient(std::shared_ptr<ClientConnection> client);
    void RemoveClient(uint32_t clientId);
    bool HasClient(uint32_t clientId) const;
    std::shared_ptr<ClientConnection> GetClient(uint32_t clientId) const;

    /// Broadcast a message to all clients, optionally excluding one.
    void Broadcast(const NetworkMessage& msg, uint32_t excludeClientId = 0);

    std::vector<std::shared_ptr<ClientConnection>> GetClients() const;

private:
    std::string id_;
    std::vector<std::shared_ptr<ClientConnection>> clients_;
};

/// Main server managing sectors and client connections.
class GameServer {
public:
    GameServer(int port = 27015);

    bool IsRunning() const;
    int GetPort() const;
    size_t GetClientCount() const;
    size_t GetSectorCount() const;

    void Start();
    void Stop();

    /// Create a new client connection and register it.
    std::shared_ptr<ClientConnection> ConnectClient(const std::string& name = "");

    /// Disconnect and remove a client by id.
    void DisconnectClient(uint32_t clientId);

    /// Look up a client by id. Returns nullptr if not found.
    std::shared_ptr<ClientConnection> GetClient(uint32_t clientId) const;

    /// Get or create a sector by id.
    std::shared_ptr<SectorServer> GetOrCreateSector(const std::string& sectorId);

    /// Get a sector by id. Returns nullptr if not found.
    std::shared_ptr<SectorServer> GetSector(const std::string& sectorId) const;

    /// Route a message from a client through the server.
    void ProcessMessage(uint32_t clientId, const NetworkMessage& msg);

    /// Tick all sectors and process queued messages.
    void Update(float dt);

private:
    int port_;
    bool running_;
    uint32_t nextClientId_;
    std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> clients_;
    std::unordered_map<std::string, std::shared_ptr<SectorServer>> sectors_;
};

} // namespace subspace
