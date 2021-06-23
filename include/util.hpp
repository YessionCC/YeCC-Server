#pragma once
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdint.h>
#include<pthread.h>
#include<unistd.h>

namespace yecc {

  pid_t GetThreadId();
  uint32_t GetFiberId();

}