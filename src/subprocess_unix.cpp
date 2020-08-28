

#include "subprocess_unix.h"

#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

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
// Android has no pthread_cancel :(
#ifdef __ANDROID__
  pthread_kill(m_waitingThread, SIGUSR1);
#else
  pthread_cancel(m_waitingThread);
#endif
  pthread_join(m_waitingThread, nullptr);
}

#ifdef __ANDROID__
void thread_exit_handler(int sig) {
  pthread_exit(0);
}
#endif

void* UnixImpl::waitForPID(void* _self)
{
#ifdef __ANDROID__
  struct sigaction actions;
  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  actions.sa_flags = 0;
  actions.sa_handler = thread_exit_handler;
  sigaction(SIGUSR1, &actions, NULL);
#endif

  UnixImpl* self = static_cast<UnixImpl*>(_self);
  waitpid(self->m_pid, NULL, 0);

  pthread_mutex_lock(&self->m_mutex);
  self->m_running = false;
  pthread_mutex_unlock(&self->m_mutex);

  return self;
}

void UnixImpl::run(commandLine_t& commandLine)
{
  const char* binary = commandLine[0];
  int pid = fork();
  switch(pid) {
    case -1:
      std::cerr << "cannot fork" << std::endl;
      break;
    case 0:
      commandLine.push_back(NULL);
      if (execvp(binary, const_cast<char* const*>(commandLine.data()))) {
        perror("Cannot launch\n");
        _exit(-1);
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
