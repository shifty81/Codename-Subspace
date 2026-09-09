#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ChatChannel { Local, System, Fleet, Corporation, Direct };
struct ChatMessage { std::uint64_t id=0; ChatChannel channel=ChatChannel::Local; std::string sender; std::string text; double timestamp=0; bool unread=true; };
class ChatCommunicationSystem {
public:
    void Post(ChatChannel channel,const std::string& sender,const std::string& text,double timestamp);
    std::vector<ChatMessage> Messages(ChatChannel channel,std::size_t limit=50) const;
    void MarkRead(ChatChannel channel);
    int Unread(ChatChannel channel) const;
private:
    std::uint64_t next_=1; std::vector<ChatMessage> messages_;
};

} // namespace subspace
