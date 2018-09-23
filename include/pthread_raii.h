#ifndef PTHREAD_RAII_H
#define PTHREAD_RAII_H

#include <pthread.h>
#include <memory>
#include <functional>
#include <utility>

namespace pthread_raii {
  class pmutex {
  public:
    pmutex() {
      pthread_mutex_init(&mutex_,nullptr);
    }
    pmutex(pmutex& other) = delete;
    ~pmutex() {
      pthread_mutex_destroy(&mutex_);
    }
    friend class plock_guard;
  private:
    pthread_mutex_t mutex_;
  };

  class plock_guard {
  public:
    plock_guard(pmutex& mutex):mutex(mutex) {
      pthread_mutex_lock(&mutex.mutex_);
    }
    plock_guard(plock_guard& other) = delete;
    ~plock_guard() {
      pthread_mutex_unlock(&mutex.mutex_);
    }
  private:
    pmutex& mutex;
  };

  class pthread {
  public:
    template<typename W>
    explicit pthread(W w) : attr_(new pthread_attr_t), thread_(new pthread_t), work_(new std::function<void()>(w)) {
      pthread_attr_init(attr_.get());
      pthread_attr_setdetachstate(attr_.get(), PTHREAD_CREATE_JOINABLE);
      pthread_create(thread_.get(),attr_.get(),dispatch, (void*)this->work_.get());
    }
    pthread(pthread&& other)=default;
    pthread(const pthread& other) = delete;
    ~pthread() {
      if(thread_) {
        pthread_join(*thread_, nullptr);
      }
    }
  private:
    std::unique_ptr<pthread_attr_t> attr_;
    std::unique_ptr<pthread_t> thread_;
    std::unique_ptr<std::function<void()>> work_;
    static void* dispatch(void* work) {
      (*(std::function<void()>*)work)();
      return nullptr;
    }
  };

}

#endif
