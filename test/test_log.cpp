#include"log.hpp"
#include"util.hpp"
#include"config.hpp"
#include"macro.hpp"
#include<iostream>
#include<thread>

void func() {
  YECC_ASSERT(false);
  YECC_LOG_DEBUG(YECC_ROOT_LOG)<<yecc::BackTraceToString(10, 0, "call: ");
}

int main() {
  func();
  return 0;
}