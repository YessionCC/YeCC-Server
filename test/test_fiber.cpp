#include"yecc.hpp"

auto logger = YECC_LOG_NAME("system");

void func(){
  YECC_LOG_DEBUG(logger)<<"fiber exec";
  yecc::Fiber::YieldToHold();
  YECC_LOG_DEBUG(logger)<<"fiber end";
}

void test_fiber() {
  yecc::Fiber::GetThis();//create main thread
  yecc::Fiber::ptr fiber(new yecc::Fiber(func));
  YECC_LOG_DEBUG(logger)<<"fiber create";
  fiber->swapIn();
  YECC_LOG_DEBUG(logger)<<"test_fiber exec after";
  fiber->swapIn();
  YECC_LOG_DEBUG(logger)<<"test_fiber end";
}

int main() {
  yecc::Thread::SetName("main");
  YAML::Node root = YAML::LoadFile("../bin/config/log.yml");
  yecc::Config::LoadFromYaml(root);
  std::vector<yecc::Thread::ptr> thrs;
  for(int i = 0; i<10; i++) {
    yecc::Thread::ptr thr(new yecc::Thread(test_fiber, "thr"+std::to_string(i)));
    thrs.push_back(thr);
  }
  for(auto& thr: thrs) {
    thr->join();
  }
  YECC_LOG_DEBUG(logger)<<"main end";
  return 0;
}