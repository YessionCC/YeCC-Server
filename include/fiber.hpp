#pragma once

#include<memory>
#include<functional>
#include<ucontext.h>
#include"thread.hpp"

namespace yecc {

  class Fiber: public std::enable_shared_from_this<Fiber> {
    public:
      typedef std::shared_ptr<Fiber> ptr;

      enum State{
        INIT,
        HOLD,
        EXEC,
        TERM, //terminate normally
        READY,
        EXCEP //terminate with exception
      };
    private:
      Fiber();

    public:
      Fiber(std::function<void()> cb, size_t stack_size = 0);
      ~Fiber();

      void reset(std::function<void()> cb);
      //swap to cur fiber and exec
      void swapIn();
      //swap cur fiber to background
      void swapOut();

      uint64_t getId() const {return m_id;}

    public:
      static Fiber::ptr GetThis();
      static void SetThis(Fiber* f);
      static void YieldToReady();
      static void YieldToHold();

      static uint64_t TotalFibers();
      static uint64_t GetCurFiberId();

      static void MainFunc();

    private:
      uint64_t m_id = 0;
      size_t m_stack_size = 0;
      State m_state = INIT;
      ucontext_t m_ctx;
      void* m_stack = nullptr;
      std::function<void()> m_cb;
  };


}