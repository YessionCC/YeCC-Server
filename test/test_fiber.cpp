#include"yecc.hpp"

void func(){
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"fiber exec";
  yecc::Fiber::YieldToHold();
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"fiber end";
}

int main() {
  {
    yecc::Fiber::GetThis();//create main thread
    yecc::Fiber::ptr fiber(new yecc::Fiber(func));
    YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"fiber create";
    fiber->swapIn();
    YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"main exec after";
    fiber->swapIn();
    YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"main end";
  }
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"main end2";
  return 0;
}