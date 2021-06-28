#include"yecc.hpp"

void func(){
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"fiber exec";
  yecc::Fiber::GetThis()->swapOut();
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"fiber end";
}

int main() {
  yecc::Fiber::ptr fiber(new yecc::Fiber(func));
  fiber->swapIn();
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"main exec after";
  fiber->swapIn();
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<"main end";
  return 0;
}