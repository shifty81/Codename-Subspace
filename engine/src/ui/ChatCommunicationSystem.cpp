#include "ui/ChatCommunicationSystem.h"
namespace subspace {
void ChatCommunicationSystem::Post(ChatChannel c,const std::string&s,const std::string&t,double ts){if(t.empty())return;messages_.push_back({next_++,c,s,t,ts,true});}
std::vector<ChatMessage> ChatCommunicationSystem::Messages(ChatChannel c,std::size_t limit) const {std::vector<ChatMessage> o;for(auto it=messages_.rbegin();it!=messages_.rend()&&o.size()<limit;++it)if(it->channel==c)o.push_back(*it);return o;}
void ChatCommunicationSystem::MarkRead(ChatChannel c){for(auto&m:messages_)if(m.channel==c)m.unread=false;}
int ChatCommunicationSystem::Unread(ChatChannel c) const {int n=0;for(const auto&m:messages_)if(m.channel==c&&m.unread)++n;return n;}
} // namespace subspace
