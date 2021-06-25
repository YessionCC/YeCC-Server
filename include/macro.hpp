#pragma once

#include<string>
#include<cassert>
#include"log.hpp"
#include"util.hpp"

#define YECC_ASSERT(x)\
  if(!(x)) {YECC_LOG_ERROR(YECC_ROOT_LOG)<<"ASSERTION: "<<#x\
    <<"\nbacktrace: \n"<<yecc::BackTraceToString(100, 2, "    ");\
    assert(x);\
  }

#define YECC_ASSERT2(x, w)\
  if(!(x)) {YECC_LOG_ERROR(YECC_ROOT_LOG)<<"ASSERTION: "<<#x\
    <<"\n"<<w\
    <<"\nbacktrace: \n"<<yecc::BackTraceToString(100, 2, "    ");\
    assert(x);\
  }
