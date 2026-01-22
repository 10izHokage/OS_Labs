#ifdef _WIN32

#include <windows.h>
#include <cstdlib>
#include "process_lib.h"

Process* start_process(const char* command) {
    if (!command) return nullptr;

    Process* proc = new Process();
    proc->exit_code = -1;

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    char* cmd = _strdup(command);
    if (!cmd) {
        delete proc;
        return nullptr;
    }

    if (!CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        free(cmd);
        delete proc;
        return nullptr;
    }

    free(cmd);
    CloseHandle(pi.hThread);
    proc->handle = pi.hProcess;
    return proc;
}

int wait_process(Process* process) {
    if (!process) return -1;

    HANDLE h = (HANDLE)process->handle;
    WaitForSingleObject(h, INFINITE);

    DWORD code = 0;
    GetExitCodeProcess(h, &code);
    process->exit_code = (int)code;
    return process->exit_code;
}

void free_process(Process* process) {
    if (process && process->handle) {
        CloseHandle((HANDLE)process->handle);
    }
    delete process;
}

#endif