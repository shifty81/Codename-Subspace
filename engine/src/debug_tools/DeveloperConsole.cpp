#include "debug_tools/DeveloperConsole.h"

#include <algorithm>
#include <sstream>

namespace subspace {

DeveloperConsole::DeveloperConsole() {
    Register("echo",[](const std::vector<std::string>& args){ConsoleResult r{true,{}};for(size_t i=0;i<args.size();++i){if(i)r.output+=' ';r.output+=args[i];}return r;});
    Register("set",[this](const std::vector<std::string>& args){if(args.size()<2)return ConsoleResult{false,"usage: set <key> <value>"};variables_[args[0]]=args[1];return ConsoleResult{true,args[0]+"="+args[1]};});
    Register("get",[this](const std::vector<std::string>& args){if(args.empty())return ConsoleResult{false,"usage: get <key>"};auto it=variables_.find(args[0]);return it==variables_.end()?ConsoleResult{false,"not set"}:ConsoleResult{true,it->second};});
    Register("help",[this](const std::vector<std::string>&){auto names=Commands();std::string out;for(size_t i=0;i<names.size();++i){if(i)out+=" ";out+=names[i];}return ConsoleResult{true,out};});
}

void DeveloperConsole::Register(const std::string& name, Command command){if(!name.empty()&&command)commands_[name]=std::move(command);}
bool DeveloperConsole::HasCommand(const std::string& name) const{return commands_.find(name)!=commands_.end();}

std::vector<std::string> DeveloperConsole::Tokenize(const std::string& line){std::vector<std::string> out;std::string cur;bool quote=false;for(char c:line){if(c=='"'){quote=!quote;continue;}if(!quote&&(c==' '||c=='\t')){if(!cur.empty()){out.push_back(cur);cur.clear();}}else cur+=c;}if(!cur.empty())out.push_back(cur);return out;}

ConsoleResult DeveloperConsole::Execute(const std::string& line){if(line.empty())return {false,"empty command"};history_.push_back(line);auto tokens=Tokenize(line);if(tokens.empty())return {false,"empty command"};auto it=commands_.find(tokens[0]);if(it==commands_.end())return {false,"unknown command: "+tokens[0]};tokens.erase(tokens.begin());return it->second(tokens);}

std::vector<std::string> DeveloperConsole::Commands() const{std::vector<std::string> out;out.reserve(commands_.size());for(const auto& p:commands_)out.push_back(p.first);std::sort(out.begin(),out.end());return out;}

} // namespace subspace
