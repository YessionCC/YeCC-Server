#include"util.hpp"
#include"log.hpp"
#include<execinfo.h>

namespace yecc {

  auto logger = YECC_LOG_NAME("system");

  pid_t GetThreadId() {
    return syscall(SYS_gettid);
  }

  uint32_t GetFiberId() {
    return 0;
  }

  void BackTrace(std::vector<std::string>& bt, int _size, int skip){
    //trace error, get called stack to trace where error is
    void** arr = (void**) malloc(_size*sizeof(void*));
    int s = backtrace(arr, _size);
    char** strs = backtrace_symbols(arr, _size);
    if(strs == nullptr) {
      YECC_LOG_ERROR(logger)<<"backtrace system error!";
      return;
    }
    for(int i = skip; i<s; i++) {
      bt.push_back(strs[i]);
    }
    free(arr);
    free(strs);
  }
  std::string BackTraceToString(int _size, int skip, const std::string& prefix){
    std::vector<std::string> strs;
    BackTrace(strs, _size, skip);
    std::stringstream ss;
    for(auto& str: strs) {
      ss<<prefix<<str<<std::endl;
    }
    return ss.str();
  }


}