#include"config.hpp"
#include"log.hpp"
#include"yaml-cpp/yaml.h"

int main() {

  YAML::Node root = YAML::LoadFile("../bin/config/log.yml");
  yecc::Config::LoadFromYaml(root);
  auto log1 = YECC_LOG_NAME("root");
  auto log2 = YECC_LOG_NAME("system");
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<log1->ToYamlStr();
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<log2->ToYamlStr();
  log1->setFormatter("%d%d%d%d");
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<log1->ToYamlStr();
  return 0;
}