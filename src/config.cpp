#include"config.hpp"
#include<list>

namespace yecc {

  Config::ConfigVarMap Config::s_data;

  static void ListAllMember(// normal static func can only call in current file
    const std::string& prefix, 
    const YAML::Node& node,
    std::list<std::pair<std::string, const YAML::Node> >& output
  ) {
    if(!prefix.empty() && 
    prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
    != std::string::npos) {
      YECC_LOG_ERROR(YECC_ROOT_LOG)<<"Config Invalid name: "<<prefix<<" : "<<node;
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()) {
      for(auto& p: node) {
        std::string nxt = prefix.empty()? 
          p.first.Scalar():
          prefix+"."+p.first.Scalar();
        ListAllMember(nxt, p.second, output);
      }
    }
  }

  void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node> > all_nodes;
    ListAllMember("", root, all_nodes);
    for(auto& p: all_nodes) {
      std::string key = p.first;
      if(key.empty()) continue;
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);
      ConfigVarBase::ptr var = LookupBase(key);
      if(var) {
        if(p.second.IsScalar())
          var->fromString(p.second.Scalar());
        else {
          std::stringstream ss;
          ss << p.second;
          var->fromString(ss.str());
        }
      }
    }
  }


}