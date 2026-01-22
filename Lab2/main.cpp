#include <iostream>
#include "lib/process_lib.h"

#ifdef _WIN32
#define CMD(x) ("cmd.exe /c " x)
#else
#define CMD(x) x
#endif

void run_test(const char* title, const char* command) {
    std::cout << "\n" << title << std::endl;

    Process* p = start_process(command);
    if (!p) {
        std::cout << "Failed to start process" << std::endl;
        return;
    }

    int code = wait_process(p);
    std::cout << "Exit code: " << code << std::endl;

    free_process(p);
}

int main() {
    std::cout << "Process library test\n";

    run_test("Test 1: Simple command", CMD("echo Hello, World!"));

#ifdef _WIN32
    run_test("Test 2: Delay command", CMD("timeout 2"));
#else
    run_test("Test 2: Delay command", CMD("sleep 2"));
#endif

    run_test("Test 3: Invalid command", "this_command_does_not_exist");

#ifdef _WIN32
    run_test("Test 4: Directory listing", CMD("dir"));
#else
    run_test("Test 4: Directory listing", CMD("ls -la"));
#endif

#ifdef _WIN32
    run_test("Test 5: Multiple commands", CMD("cd . && echo Current dir && dir"));
#else
    run_test("Test 5: Multiple commands", CMD("pwd && echo Current dir && ls"));
#endif

    std::cout << "\nAll tests completed." << std::endl;
    return 0;
}
