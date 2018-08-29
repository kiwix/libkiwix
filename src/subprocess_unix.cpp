

#include "subprocess_unix.h"

#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>

UnixImpl::UnixImpl():
  m_pid(0),
  m_running(false),
  m_mutex(PTHREAD_MUTEX_INITIALIZER),
  m_waitingThread()
{
}

UnixImpl::~UnixImpl()
{
  kill();
  pthread_cancel(m_waitingThread);
}

void* UnixImpl::waitForPID(void* _self)
{
  UnixImpl* self = static_cast<UnixImpl*>(_self);
  waitpid(self->m_pid, NULL, WEXITED);

  pthread_mutex_lock(&self->m_mutex);
  self->m_running = false;
  pthread_mutex_unlock(&self->m_mutex);

  return self;
}

void UnixImpl::run(const commandLine_t& commandLine)
{
  const char* binary = commandLine[0];
  std::cerr << "running " << binary << std::endl;
  int pid = fork();
  switch(pid) {
    case -1:
      std::cerr << "cannot fork" << std::endl;
      break;
    case 0:
      if (execvp(binary, const_cast<char* const*>(commandLine.data()))) {
        perror("Cannot launch\n");
        exit(-1);
      }

      break;
    default:
      m_pid = pid;
      m_running = true;
      pthread_create(&m_waitingThread, NULL, waitForPID, this);
      break;
  }
}

bool UnixImpl::kill()
{
  return (::kill(m_pid, SIGKILL) == 0);
}

bool UnixImpl::isRunning()
{
  pthread_mutex_lock(&m_mutex);
  bool ret = m_running;
  pthread_mutex_unlock(&m_mutex);
  return ret;
}
