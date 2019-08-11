

#ifndef KIWIXLIB_TOOL_LOCK_H
#define KIWIXLIB_TOOL_LOCK_H

#include <pthread.h>

namespace kiwix {

class Lock
{
  public:
    explicit Lock(pthread_mutex_t* mutex) :
      mp_mutex(mutex)
    {
      pthread_mutex_lock(mp_mutex);
    }
    ~Lock() {
      if (mp_mutex != nullptr) {
        pthread_mutex_unlock(mp_mutex);
      }
    }
    Lock(Lock && other) :
      mp_mutex(other.mp_mutex)
    {
      other.mp_mutex = nullptr;
    }
    Lock & operator=(Lock && other)
    {
      mp_mutex = other.mp_mutex;
      other.mp_mutex = nullptr;
      return *this;
    }


  private:
    pthread_mutex_t* mp_mutex;

    Lock(Lock const &) = delete;
    Lock & operator=(Lock const &) = delete;
};


}

#endif //KIWIXLIB_TOOL_LOCK_H
