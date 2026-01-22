#ifndef _WIN32

#include <unistd.h>
#include <sys/wait.h>
#include "process_lib.h"

Process* start_process(const char* command) {
    if (!command) return nullptr;

    Process* proc = new Process();
    proc->exit_code = -1;

    pid_t pid = fork();
    if (pid < 0) {
        delete proc;
        return nullptr;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, nullptr);
        _exit(1);
    }

    proc->handle = (void*)pid;
    return proc;
}

int wait_process(Process* process) {
    if (!process) return -1;

    int status;
    pid_t pid = (pid_t)process->handle;

    if (waitpid(pid, &status, 0) < 0)
        return -1;

    if (WIFEXITED(status)) {
        process->exit_code = WEXITSTATUS(status);
        return process->exit_code;
    }

    return -1;
}

void free_process(Process* process) {
    delete process;
}

#endif
