#include"config.hpp"
#include"log.hpp"
#include"yaml-cpp/yaml.h"

yecc::ConfigVar<int>::ptr int_val_config = 
  yecc::Config::Lookup("system.port", (int)8080, "system port");
// yecc::ConfigVar<float>::ptr int_f_config = 
//   yecc::Config::Lookup("system.port", (float)8080, "system port");
yecc::ConfigVar<std::vector<int> >::ptr int_vec_val_config = 
  yecc::Config::Lookup("system.int_vec", std::vector<int>{1,2,3}, "system vec");
yecc::ConfigVar<std::vector<std::set<int> > >::ptr int_vecvec_config = 
  yecc::Config::Lookup("system.int_vec_set", 
  std::vector<std::set<int> >{{10, 20},{2,8},{3,9}}, "system vec set");
yecc::ConfigVar<std::map<std::string, int> >::ptr int_map_val_config = 
  yecc::Config::Lookup("system.int_str_map", 
  std::map<std::string, int>{{"aa", 26},{"bb", 27},{"ss", 28}}, "system vec");

void print_test(const YAML::Node& node, int depth) {
  if(node.IsScalar()) {
    YECC_LOG_INFO(YECC_ROOT_LOG)<<std::string(depth*2, ' ')
      <<node.Scalar()<<" - "<<node.Type()<<" - "<<depth;
  }
  else if(node.IsNull()){
    YECC_LOG_INFO(YECC_ROOT_LOG)<<std::string(depth*2, ' ')
      <<"Null - "<<node.Type()<<" - "<<depth;
  }
  else if(node.IsMap()) {
    for(auto& p: node) {
      YECC_LOG_INFO(YECC_ROOT_LOG)<<std::string(depth*2, ' ')
        <<p.first.Scalar()<<" - "<<p.second.Type()<<" - "<<depth;
      print_test(p.second, depth+1);
    }
  }
  else if(node.IsSequence()) {
    int cnt = 0;
    for(auto& p: node) {
      YECC_LOG_INFO(YECC_ROOT_LOG)<<std::string(depth*2, ' ')
        <<cnt++<<" - "<<p.Type()<<" - "<<depth;
      print_test(p, depth+1);
    }
  }
}

void test_yaml() {
  YAML::Node root = YAML::LoadFile("../bin/config/test.yml");
  print_test(root, 0);
  //YECC_LOG_INFO(YECC_ROOT_LOG)<<root;
}

void test_config() {
  YECC_LOG_INFO(YECC_ROOT_LOG)<<"before "<<int_val_config->getValue();
  auto vec = int_map_val_config->getValue();
  for(auto& d: vec) {
    YECC_LOG_INFO(YECC_ROOT_LOG)<<"before "<< d.first<<" "<<d.second;
  }

  int_val_config->addListener(
    [](const int& o_v, const int& n_v){
      YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"lalaal: "<<o_v<<" - "<<n_v;
  });
  
  YAML::Node root = YAML::LoadFile("../bin/config/test.yml");
  yecc::Config::LoadFromYaml(root);
  
  YECC_LOG_INFO(YECC_ROOT_LOG)<<"after "<<int_val_config->getValue();
  vec = int_map_val_config->getValue();
  for(auto& d: vec) {
    YECC_LOG_INFO(YECC_ROOT_LOG)<<"after "<< d.first<<" "<<d.second;
  }
  YECC_LOG_INFO(YECC_ROOT_LOG)<<int_map_val_config->toString();
}

int main() {
  //YECC_LOG_INFO(YECC_ROOT_LOG)<<int_val_config->toString();
  //test_yaml();
  test_config();
  return 0;
}