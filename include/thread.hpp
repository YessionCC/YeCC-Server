#pragma once

#include<iostream>
#include<functional>
#include<memory>
#include<sys/types.h>
#include<pthread.h>
#include<semaphore.h>

namespace yecc {

  class Semaphore {
    public:
      Semaphore(uint32_t count = 0);
      ~Semaphore();

      void wait();
      void notify();
    private:
      Semaphore(const Semaphore&) = delete;
      Semaphore(const Semaphore&&) = delete;
      Semaphore& operator=(const Semaphore&) = delete;
      Semaphore& operator=(const Semaphore&&) = delete;

    private:
      sem_t m_semaphore;
  };

  template<class T>
  class ScopedMutex{
    public:
      ScopedMutex(T& mutex):m_mutex(mutex) {
        mutex.lock();
        m_locked = true;
      }
      ~ScopedMutex() {
        unlock();
      }
      void lock(){
        if(!m_locked) {
          m_mutex.lock();
          m_locked = true;
        }
      }
      void unlock(){
        if(m_locked) {
          m_mutex.unlock();
          m_locked = false;
        }
      }
    private:
      bool m_locked;
      T& m_mutex;
  };

  template<class T>
  class ReadScopedMutex{
    public:
      ReadScopedMutex(T& mutex):m_mutex(mutex) {
        mutex.rdlock();
        m_locked = true;
      }
      ~ReadScopedMutex() {
        unlock();
      }
      void lock(){
        if(!m_locked) {
          m_mutex.rdlock();
          m_locked = true;
        }
      }
      void unlock(){
        if(m_locked) {
          m_mutex.unlock();
          m_locked = false;
        }
      }
    private:
      bool m_locked;
      T& m_mutex;
  };

  template<class T>
  class WriteScopedMutex{
    public:
      WriteScopedMutex(T& mutex):m_mutex(mutex) {
        mutex.wrlock();
        m_locked = true;
      }
      ~WriteScopedMutex() {
        unlock();
      }
      void lock(){
        if(!m_locked) {
          m_mutex.wrlock();
          m_locked = true;
        }
      }
      void unlock(){
        if(m_locked) {
          m_mutex.unlock();
          m_locked = false;
        }
      }
    private:
      bool m_locked;
      T& m_mutex;
  };
  

  class Mutex {
    public:
      typedef ScopedMutex<Mutex> Lock;
      Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
      }
      ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
      }
      void lock(){
        pthread_mutex_lock(&m_mutex);
      }
      void unlock(){
        pthread_mutex_unlock(&m_mutex);
      }
    private:
      pthread_mutex_t m_mutex;
  };

  class SpinMutex {
    /*based on CAS
      busy query to test if locked
      high cpu utilization
      but will not trap in core
      suitable for thread high conflict
    */
    public:
      typedef ScopedMutex<SpinMutex> Lock;
      SpinMutex() {
        pthread_spin_init(&m_mutex, 0);
      }
      ~SpinMutex() {
        pthread_spin_destroy(&m_mutex);
      }
      void lock(){
        pthread_spin_lock(&m_mutex);
      }
      void unlock(){
        pthread_spin_unlock(&m_mutex);
      }
    private:
      pthread_spinlock_t m_mutex;
  };

  class NullMutex {//for debug
    public:
      typedef ScopedMutex<NullMutex> Lock;
      void lock(){}
      void unlock(){}
  };

  class RWMutex{
    public:
      typedef ReadScopedMutex<RWMutex> ReadLock;
      typedef WriteScopedMutex<RWMutex> WriteLock;
      RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
      }
      ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
      }
      void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
      }
      void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
      }
      void unlock() {
        pthread_rwlock_unlock(&m_lock);
      }
    private:
      pthread_rwlock_t m_lock;
  };

  class NullRWMutex {//for debug
    public:
      typedef ReadScopedMutex<NullRWMutex> ReadLock;
      typedef WriteScopedMutex<NullRWMutex> WriteLock;
      void rdlock(){}
      void wrlock(){}
      void unlock(){}
  };

  class Thread{
    public:
      typedef std::shared_ptr<Thread> ptr;

      Thread(std::function<void()> cb, const std::string& name);
      ~Thread();

      pid_t getID() const {return m_id;}
      const std::string& getName() const {return m_name;}

      void join();
      static Thread* GetThis();
      static const std::string& GetName();
      static void SetName(const std::string& name);
      static void* run(void* args);

    private:
      Thread(const Thread&) = delete;
      Thread(const Thread&&) = delete;
      Thread& operator=(const Thread&) = delete;
      Thread& operator=(const Thread&&) = delete;
      //delete all copy constructor

    private:
      pid_t m_id = -1;
      pthread_t m_thread = 0;
      std::function<void()> m_cb;
      std::string m_name;

      Semaphore m_semaphore;
  };

}