#include"fiber.hpp"
#include"config.hpp"
#include"macro.hpp"
#include<atomic>

namespace yecc {

  static std::atomic<uint64_t> g_fiber_id{0};
  static std::atomic<uint64_t> g_fiber_count{0};
  static thread_local Fiber* main_fiber = nullptr;
  static thread_local Fiber::ptr main_s_fiber = nullptr;

  static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup("fiber.stack_size", (uint32_t)1024*1024, "fiber static size");

  class StackMallocator {
    public:
      static void* Alloc(size_t _size){
        return malloc(_size);
      }

      static void Free(void* p, size_t _size){
        free(p);
      }
  };

  Fiber::Fiber(){
    m_state = EXEC;
    SetThis(this);
    if(getcontext(& m_ctx)) {
      YECC_ASSERT2(false, "getcontext");
    }
    ++g_fiber_count;
  }
  //if set stack_size 0, stack_size will be config stack size
  Fiber::Fiber(std::function<void()> cb, size_t stack_size):
   m_id(++g_fiber_id),m_cb(cb){
     ++g_fiber_count;
     m_stack_size = stack_size? stack_size: g_fiber_stack_size->getValue();
  }
  Fiber::~Fiber(){

  }

  void Fiber::reset(std::function<void()> cb){

  }
  //swap to cur fiber and exec
  void Fiber::swapIn(){

  }
  //swap cur fiber to background
  void Fiber::swapOut(){

  }

  Fiber::ptr Fiber::GetThis(){

  }
  void Fiber::SetThis(Fiber* f){

  }
  void Fiber::YieldToReady(){

  }
  void Fiber::YieldToHold(){

  }

  uint64_t Fiber::TotalFibers(){

  }

  void Fiber::MainFunc(){

  }
}