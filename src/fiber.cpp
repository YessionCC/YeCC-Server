#include"fiber.hpp"
#include"config.hpp"
#include"macro.hpp"
#include<atomic>
#include<exception>

namespace yecc {

  static std::atomic<uint64_t> g_fiber_id{0};
  static std::atomic<uint64_t> g_fiber_count{0};
  //cur fiber that was exec
  static thread_local Fiber* cur_fiber = nullptr;
  //always the main fiber
  static thread_local Fiber::ptr last_s_fiber = nullptr;

  static auto logger = YECC_LOG_NAME("system");

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
  Fiber::Fiber(std::function<void()> cb, size_t stack_size)
   :m_id(++g_fiber_id),m_cb(cb){
    
    ++g_fiber_count;
    m_stack_size = stack_size? stack_size: g_fiber_stack_size->getValue();
    m_stack = StackMallocator::Alloc(m_stack_size);
    if(getcontext(&m_ctx)) {
      YECC_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;

    makecontext(&m_ctx, Fiber::MainFunc, 0);
  }
  Fiber::~Fiber(){
    --g_fiber_count;
    if(m_stack) {//real fiber
      YECC_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEP);
      StackMallocator::Free(m_stack, m_stack_size);
    }
    else {//main fiber(thread)
      YECC_ASSERT(!m_cb);
      YECC_ASSERT(m_state == EXEC);
      Fiber* cur = cur_fiber;
      if(cur == this) {
        SetThis(nullptr);
      }
    }
  }

  void Fiber::reset(std::function<void()> cb){
    YECC_ASSERT(m_stack);
    YECC_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEP);
    m_cb = cb;
    if(getcontext(& m_ctx)) {
      YECC_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;

    makecontext(&m_ctx, Fiber::MainFunc, 0);
    m_state = INIT;
  }
  //swap to cur fiber and exec
  void Fiber::swapIn(){
    YECC_ASSERT(m_state != EXEC);
    m_state = EXEC;
    SetThis(this);

    if(swapcontext(&(last_s_fiber->m_ctx), &m_ctx)) {
      YECC_ASSERT2(false, "swapcontext");
    }
  }
  //swap cur fiber to background
  void Fiber::swapOut(){
    SetThis(last_s_fiber.get());
    if(swapcontext(&m_ctx, &(last_s_fiber->m_ctx))) {
      YECC_ASSERT2(false, "swapcontext");
    }
  }
  /*
  if in main fiber(thread), then create main fiber and set and return
  if in a certain fiber, return the fiber
  */
  Fiber::ptr Fiber::GetThis(){
    if(cur_fiber) {
      return cur_fiber->shared_from_this();
    }
    Fiber::ptr fiber(new Fiber);
    YECC_ASSERT(fiber.get() == cur_fiber)
    last_s_fiber = fiber;
    return cur_fiber->shared_from_this();
  }
  void Fiber::SetThis(Fiber* f){
    cur_fiber = f;
  }
  void Fiber::YieldToReady(){
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
  }
  void Fiber::YieldToHold(){
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
  }

  uint64_t Fiber::TotalFibers(){
    return g_fiber_count;
  }
  uint64_t Fiber::GetCurFiberId(){
    if(cur_fiber) {
      return cur_fiber->m_id;
    }
    return 0;
  }

  void Fiber::MainFunc(){
    Fiber::ptr cur = GetThis();
    YECC_ASSERT(cur);
    try{
      cur->m_cb();
      cur->m_cb = nullptr;
      cur->m_state = TERM;
    }catch(std::exception& e) {
      cur->m_state = EXCEP;
      YECC_LOG_ERROR(logger)<<"fiber exception: "<<e.what();
    }catch(...) {
      cur->m_state = EXCEP;
      YECC_LOG_ERROR(logger)<<"fiber exception";
    }
    auto raw_ptr = cur.get();
    cur.reset();
    //notice that swapout != return, this will not call destructors
    //we need to handle destruct carefully
    raw_ptr->swapOut();
  }
}