#pragma once

#include<memory>
#include<vector>
#include<list>
#include"thread.hpp"
#include"fiber.hpp"

namespace yecc {
  class Scheduler{
    public:
      typedef std::shared_ptr<Scheduler> ptr;
      typedef Mutex MutexType;

      Scheduler(size_t thead_num = 1, 
        bool use_caller = true,
        const std::string& name = "");
      virtual ~Scheduler();

      const std::string& getName() const {return m_name;}

      static Scheduler* GetThis();
      static Fiber* GetMainFiber();

      void start();
      void stop();

      template<class FiberOrCb>
      void schedule(FiberOrCb fc, pthread_t thr = -1) {
        bool need_tickle = false;
        {
          MutexType::Lock lock(m_mutex);
          need_tickle = scheduleNoBlock(fc, thr);
        }
        if(need_tickle) {
          tickle();
        }
      }

      template<class FiberOrCbBatch>
      void schedule(FiberOrCbBatch fc_b, pthread_t thr = -1) {
        bool need_tickle = false;
        {
          MutexType::Lock lock(m_mutex);
          for(auto& fc: fc_b) {
            need_tickle |= scheduleNoBlock(fc, thr);
          }
        }
        if(need_tickle) {
          tickle();
        }
      }

    protected:
      virtual void tickle();
      void run();

    private:
      template<class FiberOrCb>
      void scheduleNoBlock(FiberOrCb fc, pthread_t thr = -1) {
        bool need_tickle = m_fibers.empty();
        FiberOrFunc ft(fc, thr);
        if(ft.fiber || ft.cb) {
          m_fibers.push_back(ft);
        }
        return need_tickle;
      }

    private:
      struct FiberOrFunc {
        Fiber::ptr fiber;
        std::function<void()> cb;
        pthread_t thread;
        FiberOrFunc(Fiber::ptr f, pthread_t thr): fiber(f), thread(thr){}
        FiberOrFunc(Fiber::ptr* f, pthread_t thr): thread(thr){fiber.swap(*f);}
        FiberOrFunc(std::function<void()> f, pthread_t thr): cb(f), thread(thr){}
        FiberOrFunc(std::function<void()>* f, pthread_t thr): thread(thr){
          cb.swap(*f);
        }
        FiberOrFunc():thread(-1) {}
        void reset() {fiber = nullptr; cb = nullptr; thread = -1;}
      };
    private:
      MutexType m_mutex;
      std::vector<Scheduler::ptr> m_threads;
      std::list<FiberOrFunc> m_fibers;
      std::string m_name;
      Fiber::ptr m_mainFiber;
    protected:
      std::vector<pthread_t> m_tids;
      size_t m_totThreadCount = 0;
      size_t m_activeThreadCount = 0;
      size_t m_idleThreadCount = 0;
      pthread_t m_mainThread = 0;
      bool m_stopping = true;
      bool m_autoStop = false;

  };
}