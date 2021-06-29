#include"scheduler.hpp"
#include"log.hpp"
#include"macro.hpp"

namespace yecc {

  static auto logger = YECC_LOG_NAME("system");
  static thread_local Scheduler* t_scheduler = nullptr;
  static thread_local Fiber* t_fiber = nullptr;

  Scheduler::Scheduler(
    size_t thread_num, 
    bool use_caller,
    const std::string& name): m_name(name) {
    YECC_ASSERT(thread_num>0);

    if(use_caller) {
      Fiber::GetThis();
      --thread_num;
      YECC_ASSERT(GetThis() == nullptr);
      t_scheduler = this;

      // std::bind transform class func to callable func
      m_mainFiber.reset(new Fiber(std::bind(Scheduler::run, this)));
      Thread::SetName(m_name);

      t_fiber = m_mainFiber.get();
      m_mainThread = GetThreadId();
      m_tids.push_back(m_mainThread);
    }
    else {
      m_mainThread = -1;
    }
    m_totThreadCount = thread_num;
  }
  Scheduler::~Scheduler(){

  }

  Scheduler* Scheduler::GetThis(){

  }
  Fiber* Scheduler::GetMainFiber(){

  }

  void Scheduler::start(){

  }
  void Scheduler::stop(){

  }
  void Scheduler::run(){

  }
}