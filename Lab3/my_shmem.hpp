#pragma once

#include <string.h>   
#include <stdlib.h>   
#include <iostream>

#if defined(_WIN32)
#   include <windows.h>
#   define MAP_NAME_PREFIX "Local\\"
#   define INV_HANDLE      (NULL)
#   define CSEM            HANDLE
#   define HANDLE_T        HANDLE
#else
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <semaphore.h>
#   define MAP_NAME_PREFIX "/"
#   define INV_HANDLE      (-1)
#   define CSEM            sem_t*
#   define HANDLE_T        int
#endif

#define SEM_NAME_POSTFIX "_sem"

namespace cplib
{
    template <class T>
    class SharedMem
    {
    public:
        SharedMem(const char* name, bool create_if_not_exists = true)
            : _mem(NULL), _sem(NULL), _fd(INV_HANDLE), _fname(NULL), _semname(NULL)
        {
            _fname = (char*)malloc(strlen(name) + strlen(MAP_NAME_PREFIX) + 1);
            memcpy(_fname, MAP_NAME_PREFIX, strlen(MAP_NAME_PREFIX));
            memcpy(_fname + strlen(MAP_NAME_PREFIX), name, strlen(name) + 1);

            _semname = (char*)malloc(strlen(_fname) + strlen(SEM_NAME_POSTFIX) + 1);
            memcpy(_semname, _fname, strlen(_fname));
            memcpy(_semname + strlen(_fname), SEM_NAME_POSTFIX, strlen(SEM_NAME_POSTFIX) + 1);

            bool is_new = false;

            bool ok = OpenMem(_fname, _semname);
            if (!ok && create_if_not_exists) {
                ok = CreateMem(_fname, _semname);
                if (ok) is_new = true;
            }

            if (ok) ok = MapMem();

            if (ok && is_new) {
                _mem->cnt = 0;
                _mem->str = T();
            }

            if (ok) {
                LockSema();
                _mem->cnt++;
                UnlockSema();
            } else {
                if (is_new) DestroyMem();
                else CloseMem();
            }
        }

        SharedMem(const std::string& name, bool create_if_not_exists = true)
            : SharedMem(name.c_str(), create_if_not_exists) {}

        virtual ~SharedMem()
        {
            if (IsValid()) {
                int cnt = 0;
                LockSema();
                _mem->cnt--;
                cnt = _mem->cnt;
                UnlockSema();

                if (cnt <= 0) DestroyMem();
                else CloseMem();
            }

            if (_fname)   free(_fname);
            if (_semname) free(_semname);
        }

        bool IsValid() const { return (_fd != INV_HANDLE && _sem != NULL && _mem != NULL); }

        void Lock()   { LockSema(); }
        void Unlock() { UnlockSema(); }

        T* Data()
        {
            if (!IsValid()) return NULL;
            return &_mem->str;
        }

    private:
        bool OpenMem(const char* mem_name, const char* sem_name)
        {
#if defined(_WIN32)
            _fd = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, mem_name);
            if (_fd != INV_HANDLE) {
                _sem = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, sem_name);
            }
#else
            _fd = shm_open(mem_name, O_RDWR, 0644);
            if (_fd != INV_HANDLE) {
                _sem = sem_open(sem_name, 0);
                if (_sem == SEM_FAILED) _sem = NULL;
            }
#endif
            return (_fd != INV_HANDLE && _sem != NULL);
        }

        bool CreateMem(const char* mem_name, const char* sem_name)
        {
#if defined(_WIN32)
            _fd = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                     (DWORD)sizeof(shmem_contents), mem_name);
            if (_fd != INV_HANDLE) {
                _sem = CreateSemaphoreA(NULL, 1, 1, sem_name);
            }
#else
            _fd = shm_open(mem_name, O_CREAT | O_EXCL | O_RDWR, 0644);
            if (_fd != INV_HANDLE) {
                (void)ftruncate(_fd, sizeof(shmem_contents));
                _sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
                if (_sem == SEM_FAILED) _sem = NULL;
            }
#endif
            return (_fd != INV_HANDLE && _sem != NULL);
        }

        bool MapMem()
        {
            if (_fd == INV_HANDLE) return false;

#if defined(_WIN32)
            _mem = reinterpret_cast<shmem_contents*>(
                MapViewOfFile(_fd, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(shmem_contents))
            );
#else
            void* res = mmap(NULL, sizeof(shmem_contents),
                             PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
            if (res == MAP_FAILED) _mem = NULL;
            else _mem = reinterpret_cast<shmem_contents*>(res);
#endif
            return (_mem != NULL);
        }

        bool UnMapMem()
        {
            if (_mem == NULL) return false;

#if defined(_WIN32)
            UnmapViewOfFile(_mem);
#else
            munmap(_mem, sizeof(shmem_contents));
#endif
            _mem = NULL;
            return true;
        }

        void CloseMem()
        {
            UnMapMem();

            if (_fd != INV_HANDLE) {
#if defined(_WIN32)
                CloseHandle(_fd);
#else
                close(_fd);
#endif
                _fd = INV_HANDLE;
            }

            if (_sem != NULL) {
#if defined(_WIN32)
                CloseHandle(_sem);
#else
                sem_close(_sem);
#endif
                _sem = NULL;
            }
        }

        void DestroyMem()
        {
            CloseMem();
			
#if !defined(_WIN32)
            shm_unlink(_fname);
            sem_unlink(_semname);
#endif
        }

        void LockSema()
        {
#if defined(_WIN32)
            WaitForSingleObject(_sem, INFINITE);
#else
            sem_wait(_sem);
#endif
        }

        void UnlockSema()
        {
#if defined(_WIN32)
            ReleaseSemaphore(_sem, 1, NULL);
#else
            sem_post(_sem);
#endif
        }

    private:
        struct shmem_contents
        {
            T   str;
            int cnt;
        } *_mem;

        CSEM     _sem;
        HANDLE_T _fd;

        char* _fname;
        char* _semname;
    };
}
