#ifndef KIWIX_SUBPROCESS_UNIX_H_
#define KIWIX_SUBPROCESS_UNIX_H_

#include "subprocess.h"

#include <pthread.h>


class UnixImpl : public SubprocessImpl
{
  private:
    int m_pid;
    bool m_running;
    pthread_mutex_t m_mutex;
    pthread_t m_waitingThread;

  public:
    UnixImpl();
    virtual ~UnixImpl();

    void run(commandLine_t& commandLine);
    bool kill();
    bool isRunning();

    static void* waitForPID(void* self);
};

#endif //KIWIX_SUBPROCESS_UNIX_H_
