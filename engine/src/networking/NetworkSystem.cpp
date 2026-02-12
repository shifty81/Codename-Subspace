#include "networking/NetworkSystem.h"

#include <chrono>

namespace subspace {

// --- NetworkMessage ---

NetworkMessage::NetworkMessage()
    : type(MessageType::ChatMessage),
      timestamp(std::chrono::duration<double>(
          std::chrono::system_clock::now().time_since_epoch()).count()) {}

NetworkMessage::NetworkMessage(MessageType t, const std::string& d)
    : type(t), data(d),
      timestamp(std::chrono::duration<double>(
          std::chrono::system_clock::now().time_since_epoch()).count()) {}

std::vector<uint8_t> NetworkMessage::Serialize() const {
    // Format: [type:1][dataLen:4][data:N][timestamp:8]
    uint32_t dataLen = static_cast<uint32_t>(data.size());
    std::vector<uint8_t> bytes(1 + 4 + dataLen + 8);

    size_t offset = 0;
    bytes[offset] = static_cast<uint8_t>(type);
    offset += 1;

    std::memcpy(bytes.data() + offset, &dataLen, sizeof(dataLen));
    offset += sizeof(dataLen);

    if (dataLen > 0) {
        std::memcpy(bytes.data() + offset, data.data(), dataLen);
        offset += dataLen;
    }

    std::memcpy(bytes.data() + offset, &timestamp, sizeof(timestamp));

    return bytes;
}

NetworkMessage NetworkMessage::Deserialize(const std::vector<uint8_t>& bytes) {
    NetworkMessage msg;
    if (bytes.size() < 1 + 4 + 8) return msg;

    size_t offset = 0;
    msg.type = static_cast<MessageType>(bytes[offset]);
    offset += 1;

    uint32_t dataLen = 0;
    std::memcpy(&dataLen, bytes.data() + offset, sizeof(dataLen));
    offset += sizeof(dataLen);

    if (offset + dataLen + sizeof(double) > bytes.size()) return msg;

    if (dataLen > 0) {
        msg.data.assign(reinterpret_cast<const char*>(bytes.data() + offset), dataLen);
        offset += dataLen;
    }

    std::memcpy(&msg.timestamp, bytes.data() + offset, sizeof(msg.timestamp));

    return msg;
}

// --- ClientConnection ---

ClientConnection::ClientConnection(uint32_t id, const std::string& name)
    : id_(id), name_(name), connected_(true) {}

uint32_t ClientConnection::GetId() const { return id_; }

const std::string& ClientConnection::GetName() const { return name_; }

bool ClientConnection::IsConnected() const { return connected_; }

void ClientConnection::Disconnect() { connected_ = false; }

void ClientConnection::QueueMessage(const NetworkMessage& msg) {
    outbox_.push_back(msg);
}

std::vector<NetworkMessage> ClientConnection::FlushOutbox() {
    std::vector<NetworkMessage> msgs = std::move(outbox_);
    outbox_.clear();
    return msgs;
}

std::vector<NetworkMessage> ClientConnection::FlushInbox() {
    std::vector<NetworkMessage> msgs = std::move(inbox_);
    inbox_.clear();
    return msgs;
}

void ClientConnection::ReceiveMessage(const NetworkMessage& msg) {
    inbox_.push_back(msg);
}

const std::string& ClientConnection::GetCurrentSector() const { return currentSector_; }

void ClientConnection::SetCurrentSector(const std::string& sector) {
    currentSector_ = sector;
}

// --- SectorServer ---

SectorServer::SectorServer(const std::string& id) : id_(id) {}

const std::string& SectorServer::GetId() const { return id_; }

size_t SectorServer::GetClientCount() const { return clients_.size(); }

void SectorServer::AddClient(std::shared_ptr<ClientConnection> client) {
    if (!client) return;
    if (!HasClient(client->GetId())) {
        clients_.push_back(std::move(client));
    }
}

void SectorServer::RemoveClient(uint32_t clientId) {
    clients_.erase(
        std::remove_if(clients_.begin(), clients_.end(),
            [clientId](const std::shared_ptr<ClientConnection>& c) {
                return c->GetId() == clientId;
            }),
        clients_.end());
}

bool SectorServer::HasClient(uint32_t clientId) const {
    return std::any_of(clients_.begin(), clients_.end(),
        [clientId](const std::shared_ptr<ClientConnection>& c) {
            return c->GetId() == clientId;
        });
}

std::shared_ptr<ClientConnection> SectorServer::GetClient(uint32_t clientId) const {
    auto it = std::find_if(clients_.begin(), clients_.end(),
        [clientId](const std::shared_ptr<ClientConnection>& c) {
            return c->GetId() == clientId;
        });
    return (it != clients_.end()) ? *it : nullptr;
}

void SectorServer::Broadcast(const NetworkMessage& msg, uint32_t excludeClientId) {
    for (auto& client : clients_) {
        if (client->GetId() != excludeClientId && client->IsConnected()) {
            client->QueueMessage(msg);
        }
    }
}

std::vector<std::shared_ptr<ClientConnection>> SectorServer::GetClients() const {
    return clients_;
}

// --- GameServer ---

GameServer::GameServer(int port)
    : port_(port), running_(false), nextClientId_(1) {}

bool GameServer::IsRunning() const { return running_; }

int GameServer::GetPort() const { return port_; }

size_t GameServer::GetClientCount() const { return clients_.size(); }

size_t GameServer::GetSectorCount() const { return sectors_.size(); }

void GameServer::Start() {
    if (running_) return;
    running_ = true;
}

void GameServer::Stop() {
    if (!running_) return;
    running_ = false;

    for (auto& [id, client] : clients_) {
        client->Disconnect();
    }
    clients_.clear();
    sectors_.clear();
}

std::shared_ptr<ClientConnection> GameServer::ConnectClient(const std::string& name) {
    if (!running_) return nullptr;

    uint32_t id = nextClientId_++;
    auto client = std::make_shared<ClientConnection>(id, name);
    clients_[id] = client;
    return client;
}

void GameServer::DisconnectClient(uint32_t clientId) {
    auto it = clients_.find(clientId);
    if (it == clients_.end()) return;

    auto& client = it->second;
    // Remove from current sector
    const auto& sector = client->GetCurrentSector();
    if (!sector.empty()) {
        auto sit = sectors_.find(sector);
        if (sit != sectors_.end()) {
            sit->second->RemoveClient(clientId);
        }
    }

    client->Disconnect();
    clients_.erase(it);
}

std::shared_ptr<ClientConnection> GameServer::GetClient(uint32_t clientId) const {
    auto it = clients_.find(clientId);
    return (it != clients_.end()) ? it->second : nullptr;
}

std::shared_ptr<SectorServer> GameServer::GetOrCreateSector(const std::string& sectorId) {
    auto it = sectors_.find(sectorId);
    if (it != sectors_.end()) return it->second;

    auto sector = std::make_shared<SectorServer>(sectorId);
    sectors_[sectorId] = sector;
    return sector;
}

std::shared_ptr<SectorServer> GameServer::GetSector(const std::string& sectorId) const {
    auto it = sectors_.find(sectorId);
    return (it != sectors_.end()) ? it->second : nullptr;
}

void GameServer::ProcessMessage(uint32_t clientId, const NetworkMessage& msg) {
    auto client = GetClient(clientId);
    if (!client) return;

    switch (msg.type) {
        case MessageType::JoinSector: {
            if (msg.data.empty()) return;

            // Leave current sector if any
            const auto& current = client->GetCurrentSector();
            if (!current.empty()) {
                auto oldSector = GetSector(current);
                if (oldSector) {
                    oldSector->RemoveClient(clientId);
                }
            }

            // Join new sector
            auto sector = GetOrCreateSector(msg.data);
            sector->AddClient(client);
            client->SetCurrentSector(msg.data);

            // Send confirmation
            client->QueueMessage(NetworkMessage(MessageType::SectorJoined, msg.data));
            break;
        }
        case MessageType::LeaveSector: {
            if (msg.data.empty()) return;

            auto sector = GetSector(msg.data);
            if (sector) {
                sector->RemoveClient(clientId);
            }
            client->SetCurrentSector("");
            break;
        }
        case MessageType::EntityUpdate: {
            const auto& sectorId = client->GetCurrentSector();
            if (sectorId.empty()) return;

            auto sector = GetSector(sectorId);
            if (sector) {
                sector->Broadcast(msg, clientId);
            }
            break;
        }
        case MessageType::ChatMessage: {
            const auto& sectorId = client->GetCurrentSector();
            if (sectorId.empty()) return;

            auto sector = GetSector(sectorId);
            if (sector) {
                sector->Broadcast(msg, clientId);
            }
            break;
        }
        default:
            break;
    }
}

void GameServer::Update(float /*dt*/) {
    if (!running_) return;

    // Process inbox messages for all connected clients
    for (auto& [id, client] : clients_) {
        if (!client->IsConnected()) continue;

        auto messages = client->FlushInbox();
        for (auto& msg : messages) {
            ProcessMessage(id, msg);
        }
    }
}

} // namespace subspace
