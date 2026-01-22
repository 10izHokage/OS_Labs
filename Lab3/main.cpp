#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <signal.h>
#endif

#include "my_shmem.hpp"

struct CounterData {
    uint32_t magic;
    int counter;
    int master_pid;
    bool copy1_running;
    bool copy2_running;
};

static constexpr uint32_t MAGIC = 0xC0FFEE01;

cplib::SharedMem<CounterData> shm("my_counter");
cplib::SharedMem<uint32_t> logSem("my_counter_log"); 

static void log_lock()  { logSem.Lock(); }
static void log_unlock(){ logSem.Unlock(); }

static int get_pid() {
#if defined(_WIN32)
    return (int)GetCurrentProcessId();
#else
    return (int)getpid();
#endif
}

static std::string current_time() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto itt = system_clock::to_time_t(now);

#if defined(_WIN32)
    tm tmv{};
    localtime_s(&tmv, &itt);
#else
    tm tmv{};
    localtime_r(&itt, &tmv);
#endif

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03lld",
        tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec,
        (long long)ms.count());
    return std::string(buf);
}

static void log_msg(const std::string& msg) {
    log_lock();
    std::ofstream log("log.txt", std::ios::app);
    if (log.is_open()) {
        log << msg << "\n";
        log.flush();
    }
    log_unlock();
}

static bool is_process_alive(int pid) {
    if (pid <= 0) return false;
#if defined(_WIN32)
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
    if (!h) return false;
    DWORD code = 0;
    BOOL ok = GetExitCodeProcess(h, &code);
    CloseHandle(h);
    if (!ok) return false;
    return code == STILL_ACTIVE;
#else
    return kill(pid, 0) == 0;
#endif
}

static void init_shared_if_needed() {
    shm.Lock();
    CounterData* d = shm.Data();
    if (d && d->magic != MAGIC) {
        std::memset(d, 0, sizeof(CounterData));
        d->magic = MAGIC;
        d->counter = 0;
        d->master_pid = 0;
        d->copy1_running = false;
        d->copy2_running = false;
    }
    shm.Unlock();
}

static void run_copy(const std::string& type) {
    int pid = get_pid();
    log_msg("Copy " + type + " started, PID=" + std::to_string(pid) + ", time=" + current_time());

    if (type == "copy1") {
        shm.Lock();
        shm.Data()->counter += 10;
        shm.Unlock();
    } else if (type == "copy2") {
        shm.Lock();
        shm.Data()->counter *= 2;
        shm.Unlock();

        std::this_thread::sleep_for(std::chrono::seconds(2));

        shm.Lock();
        shm.Data()->counter /= 2;
        shm.Unlock();
    }

    log_msg("Copy " + type + " finished, PID=" + std::to_string(pid) + ", time=" + current_time());

    shm.Lock();
    if (type == "copy1") shm.Data()->copy1_running = false;
    if (type == "copy2") shm.Data()->copy2_running = false;
    shm.Unlock();
}

static void spawn_copy(const std::string& exePath, const std::string& type) {
    int pid = get_pid();
    shm.Lock();
    if (type == "copy1") {
        if (shm.Data()->copy1_running) {
            shm.Unlock();
            log_msg("PID=" + std::to_string(pid) + " cannot spawn copy1: still running");
            return;
        }
        shm.Data()->copy1_running = true;
    } else if (type == "copy2") {
        if (shm.Data()->copy2_running) {
            shm.Unlock();
            log_msg("PID=" + std::to_string(pid) + " cannot spawn copy2: still running");
            return;
        }
        shm.Data()->copy2_running = true;
    }
    shm.Unlock();

#if defined(_WIN32)
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::string cmd = "\"" + exePath + "\" " + type;
    std::vector<char> cmd_mod(cmd.begin(), cmd.end());
    cmd_mod.push_back('\0');

    if (!CreateProcessA(NULL, cmd_mod.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        log_msg("Failed to create Windows copy process: " + type);

        shm.Lock();
        if (type == "copy1") shm.Data()->copy1_running = false;
        if (type == "copy2") shm.Data()->copy2_running = false;
        shm.Unlock();
        return;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
#else
    pid_t child = fork();
    if (child == 0) {
        run_copy(type);
        _exit(0);
    }
    if (child < 0) {
        log_msg("Failed to fork copy process: " + type);

        shm.Lock();
        if (type == "copy1") shm.Data()->copy1_running = false;
        if (type == "copy2") shm.Data()->copy2_running = false;
        shm.Unlock();
    }
#endif
}

static std::string get_exe_path(char** argv) {
#if defined(_WIN32)
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) return std::string(buf);
#endif
    return (argv && argv[0]) ? std::string(argv[0]) : std::string("lab3");
}

int main(int argc, char** argv) {
	if (!shm.IsValid()) {
        std::cout << "Failed to create shared memory block!" << std::endl;
        return -1;
    }
    if (!logSem.IsValid()) {
        std::cout << "Failed to create log semaphore block!" << std::endl;
        return -1;
    }
	
    init_shared_if_needed();
	
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "copy1" || arg == "copy2") {
            run_copy(arg);
            return 0;
        }
    }

    const int pid = get_pid();
    const std::string exePath = get_exe_path(argv);

    log_msg("Program started, PID=" + std::to_string(pid) + ", time=" + current_time());

    std::atomic<bool> running(true);

    auto elect_master = [&]() -> bool {
        shm.Lock();
        int mpid = shm.Data()->master_pid;
        shm.Unlock();

        if (mpid == 0 || !is_process_alive(mpid)) {
            shm.Lock();

            mpid = shm.Data()->master_pid;
            if (mpid == 0 || !is_process_alive(mpid)) {
                shm.Data()->master_pid = pid;
            }
            bool is_master = (shm.Data()->master_pid == pid);
            shm.Unlock();
            return is_master;
        }
        return mpid == pid;
    };

    std::thread counter_thread([&]() {
        while (running.load()) {
            shm.Lock();
            shm.Data()->counter += 1;
            shm.Unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    });

    std::thread input_thread([&]() {
        std::string token;
        while (running.load() && (std::cin >> token)) {
            if (token == "q" || token == "quit" || token == "exit") {
                running.store(false);
                break;
            }
            try {
                int val = std::stoi(token);
                shm.Lock();
                shm.Data()->counter = val;
                shm.Unlock();
            } catch (...) {
            }
        }
        running.store(false);
    });

    std::thread log_thread([&]() {
        while (running.load()) {
            bool is_master = elect_master();
            if (is_master) {
                int cnt = 0;
                shm.Lock();
                cnt = shm.Data()->counter;
                shm.Unlock();

                log_msg("PID=" + std::to_string(pid) + " Time=" + current_time() +
                        " Counter=" + std::to_string(cnt));
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::thread copy_thread([&]() {
        while (running.load()) {
            bool is_master = elect_master();
            if (is_master) {
                shm.Lock();
                bool c1 = shm.Data()->copy1_running;
                shm.Unlock();
                if (c1) {
                    log_msg("PID=" + std::to_string(pid) + " cannot spawn copy1: still running");
                } else {
                    spawn_copy(exePath, "copy1");
                }
                shm.Lock();
                bool c2 = shm.Data()->copy2_running;
                shm.Unlock();
                if (c2) {
                    log_msg("PID=" + std::to_string(pid) + " cannot spawn copy2: still running");
                } else {
                    spawn_copy(exePath, "copy2");
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    });

    counter_thread.join();
    input_thread.join();
    log_thread.join();
    copy_thread.join();

    return 0;
}
