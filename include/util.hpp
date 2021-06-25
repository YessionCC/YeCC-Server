#pragma once
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdint.h>
#include<pthread.h>
#include<unistd.h>
#include<vector>
#include<string>

namespace yecc {

  pid_t GetThreadId();
  uint32_t GetFiberId();
  void BackTrace(std::vector<std::string>& bt, int _size, int skip);
  std::string BackTraceToString(int _size, int skip, const std::string& prefix="");

}