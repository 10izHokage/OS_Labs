#ifndef PROCESS_LIB_H
#define PROCESS_LIB_H

#ifdef _WIN32
#define PROCESS_API __declspec(dllexport)
#else
#define PROCESS_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int exit_code;
    void* handle;
} Process;

PROCESS_API Process* start_process(const char* command);

PROCESS_API int wait_process(Process* process);


PROCESS_API void free_process(Process* process);

#ifdef __cplusplus
}
#endif

#endif 
