#pragma once

#include<ucontext.h>
#include<memory>
#include<functional>
#include"thread.hpp"

namespace yecc {

  class Fiber: public std::enable_shared_from_this<Fiber> {
    public:
      typedef std::shared_ptr<Fiber> ptr;

      enum State{
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY
      };
    private:
      Fiber();

    public:
      Fiber(std::function<void()> cb, size_t stack_size);
      ~Fiber();

      void reset(std::function<void()> cb);
      //swap to cur fiber and exec
      void swapIn();
      //swap cur fiber to background
      void swapOut();

    public:
      static Fiber::ptr GetThis();
      static void SetThis(Fiber* f);
      static void YieldToReady();
      static void YieldToHold();

      static uint64_t TotalFibers();

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