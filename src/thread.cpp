#include"thread.hpp"
#include"log.hpp"
#include"util.hpp"
#include<exception>

static yecc::Logger::ptr g_logger = YECC_LOG_NAME("system");

namespace yecc {

  static thread_local Thread* t_thread = nullptr;
  static thread_local std::string t_thread_name = "UNKNOWN";
  //thread_local: every thread will create instancea and will be destroy 
  //when thread destroy

  Semaphore::Semaphore(uint32_t count) {
    if(sem_init(&m_semaphore, 0, count)){
      throw std::logic_error("sem_init error");
    }
  }
  Semaphore::~Semaphore(){
    sem_destroy(&m_semaphore);
  }

  void Semaphore::wait(){
    if(sem_wait(&m_semaphore)) {
      throw std::logic_error("sem_wait error");
    }
  }
  void Semaphore::notify(){
    if(sem_post(&m_semaphore)){
      throw std::logic_error("sem_post error");
    }
  }

  Thread::Thread(std::function<void()> cb, const std::string& name):
    m_cb(cb) {
    if(name.empty()) m_name = "UNKNOWN";
    else m_name = name;
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt) {
      YECC_LOG_ERROR(g_logger)<<"thread create failed: rt = "<<rt
        <<" name = "<<m_name;
      throw std::logic_error("pthread_create error!");
    }

    m_semaphore.wait();
  }
  Thread::~Thread(){
    if(m_thread) {//detach after join will segmatation fault!
      pthread_detach(m_thread);//!!!!!
    }
  }

  void Thread::join(){
    if(m_thread) {
      int rt = pthread_join(m_thread, nullptr);
      if(rt) {
        YECC_LOG_ERROR(g_logger)<<"thread join failed: rt = "<<rt
          <<" name = "<<m_name;
        throw std::logic_error("pthread_join error!");
      }
    }
  }
  Thread* Thread::GetThis(){
    return t_thread;
  }
  const std::string& Thread::GetName(){
    return t_thread_name;
  }
  void Thread::SetName(const std::string& name){
    if(t_thread) {
      t_thread->m_name = name;
    }
    t_thread_name = name;
  }
  void* Thread::run(void* args){
    Thread* thread = (Thread*) args;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0,15).c_str());
    std::function<void()> cb;
    cb.swap(thread->m_cb);//??

    thread->m_semaphore.notify();
    //to make sure that thread init successfully and 
    //then main thread can create others
    cb();
    return 0;
  }
}