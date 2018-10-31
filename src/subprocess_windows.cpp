

#include "subprocess_windows.h"

#include <windows.h>
#include <winbase.h>
#include <iostream>
#include <sstream>

WinImpl::WinImpl():
  m_pid(0),
  m_running(false),
  m_handle(INVALID_HANDLE_VALUE)
{
  InitializeCriticalSection(&m_criticalSection);
}

WinImpl::~WinImpl()
{
  kill();
  CloseHandle(m_handle);
  DeleteCriticalSection(&m_criticalSection);
}

DWORD WINAPI WinImpl::waitForPID(void* _self)
{
  WinImpl* self = static_cast<WinImpl*>(_self);
  WaitForSingleObject(self->m_handle, INFINITE);

  EnterCriticalSection(&self->m_criticalSection);
  self->m_running = false;
  LeaveCriticalSection(&self->m_criticalSection);

  return 0;
}

std::unique_ptr<wchar_t[]> toWideChar(const std::string& value)
{
  auto size = MultiByteToWideChar(CP_UTF8, 0,
                value.c_str(), -1, nullptr, 0);
  auto wdata = std::unique_ptr<wchar_t[]>(new wchar_t[size]);
  auto ret = MultiByteToWideChar(CP_UTF8, 0,
                value.c_str(), -1, wdata.get(), size);
  if (0 == ret) {
    std::ostringstream oss;
    oss << "Cannot convert to wchar : " << GetLastError();
    throw std::runtime_error(oss.str());
  }
  return wdata;
}


void WinImpl::run(const commandLine_t& commandLine)
{
  STARTUPINFOW startInfo = {0};
  PROCESS_INFORMATION procInfo;
  startInfo.cb = sizeof(startInfo);
  std::ostringstream oss;
  for(auto& item: commandLine) {
    oss << item << " ";
  }
  auto wCommandLine = toWideChar(oss.str());
  if (CreateProcessW(
    NULL,
    wCommandLine.get(),
    NULL,
    NULL,
    false,
    CREATE_NO_WINDOW,
    NULL,
    NULL,
    &startInfo,
    &procInfo)) {
    m_pid = procInfo.dwProcessId;
    m_handle = procInfo.hProcess;
    CloseHandle(procInfo.hThread);
    m_running = true;
    CreateThread(NULL, 0, &waitForPID, this, 0, NULL	);
  }
}

bool WinImpl::kill()
{
  return TerminateProcess(m_handle, 0);
}

bool WinImpl::isRunning()
{
  EnterCriticalSection(&m_criticalSection);
  bool ret = m_running;
  LeaveCriticalSection(&m_criticalSection);
  return ret;
}
