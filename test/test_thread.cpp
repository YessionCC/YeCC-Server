#include"thread.hpp"
#include"log.hpp"
#include"config.hpp"
#include<vector>

int count = 0;
//yecc::Mutex mutex;

auto logger = YECC_LOG_NAME("system");

void func1() {
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"name: "<<yecc::Thread::GetName()
    <<" this.name: "<<yecc::Thread::GetThis()->getName()
    <<" id: "<<yecc::Thread::GetThis()->getID();
  YECC_LOG_DEBUG(logger)<<"123465789abcdefghijklmnopqrst";
}

void* func2(void*) {
  return nullptr;
}

void test() {
  YAML::Node root = YAML::LoadFile("../bin/config/log.yml");
  yecc::Config::LoadFromYaml(root);
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<logger->ToYamlStr();
  yecc::Config::Visit(
    [](yecc::ConfigVarBase::ptr p){
      YECC_LOG_DEBUG(YECC_ROOT_LOG)<<132;
      YECC_LOG_DEBUG(YECC_ROOT_LOG)<<p->toString();
  });
  yecc::Thread::ptr thr(new yecc::Thread(func1, "123"));
  yecc::Thread::ptr thr1(new yecc::Thread(func1, "456"));
  yecc::Thread::ptr thr2(new yecc::Thread(func1, "789"));
  yecc::Thread::ptr thr3(new yecc::Thread(func1, "111"));
  yecc::Thread::ptr thr4(new yecc::Thread(func1, "158"));
  yecc::Thread::ptr thr5(new yecc::Thread(func1, "16"));
  thr->join();
  thr1->join();
  thr2->join();
  thr3->join();
  thr4->join();
  thr5->join();
}

int main() {
  test();
  return 0;
}