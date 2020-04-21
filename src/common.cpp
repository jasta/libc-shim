#include <libc_shim.h>

#include <stdexcept>
#include "common.h"
#include "pthreads.h"
#include "semaphore.h"
#include "network.h"
#include "dirent.h"
#include "cstdio.h"
#include "errno.h"
#include "ctype_data.h"
#include <math.h>
#ifndef __APPLE__
#include <sys/auxv.h>
#endif

using namespace shim;

extern "C" unsigned long __umoddi3(unsigned long a, unsigned long b);
extern "C" unsigned long __udivdi3(unsigned long a, unsigned long b);
extern "C" long __divdi3(long a, long b);

extern "C" int __cxa_atexit(void (*)(void*), void*, void*);
extern "C" void __cxa_finalize(void * d);

uintptr_t bionic::stack_chk_guard = []() {
#ifndef __APPLE__
    return *((uintptr_t*) getauxval(AT_RANDOM));
#else
    return 0;
#endif
}();

int bionic::to_host_clock_type(bionic::clock_type type) {
    switch (type) {
        case clock_type::REALTIME: return CLOCK_REALTIME;
        case clock_type::MONOTONIC: return CLOCK_MONOTONIC;
        default: throw std::runtime_error("Unexpected clock type");
    }
}

void bionic::on_stack_chk_fail() {
    fprintf(stderr, "stack corruption has been detected\n");
    abort();
}

void shim::assert(const char *file, int line, const char *msg) {
    fprintf(stderr, "assert failed: %s:%i: %s\n", file, line, msg);
    abort();
}

void shim::assert2(const char *file, int line, const char *function, const char *msg) {
    fprintf(stderr, "assert failed: %s:%i %s: %s\n", file, line, function, msg);
    abort();
}

void shim::add_common_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
#ifdef __APPLE__
        {"__errno", bionic::errno_with_translation},
        {"__set_errno", bionic::set_errno_with_translation},
#else
        {"__errno", __errno_location},
        {"__set_errno", bionic::set_errno_without_translation},
#endif

        {"__stack_chk_fail", &bionic::on_stack_chk_fail},
        {"__stack_chk_guard", &bionic::stack_chk_guard},

        {"__assert", assert},
        {"__assert2", assert2},

        {"__cxa_atexit", ::__cxa_atexit},
        {"__cxa_finalize", ::__cxa_finalize}
    });
}

void shim::add_stdlib_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"abort", abort},
        {"atexit", atexit},
        {"exit", exit},
        {"_Exit", _Exit},

        {"system", system},

        {"getenv", getenv},
        {"putenv", putenv},
        {"setenv", setenv},
        {"unsetenv", unsetenv},


        {"random", random},
        {"srandom", srandom},
        {"initstate", initstate},
        {"setstate", setstate},

        {"rand", rand},
        {"srand", srand},
        {"rand_r", rand_r},
        {"drand48", drand48},
        {"erand48", erand48},
        {"lrand48", lrand48},
        {"nrand48", nrand48},
        {"mrand48", mrand48},
        {"jrand48", jrand48},
        {"srand48", srand48},
        {"seed48", seed48},
        {"lcong48", lcong48},
    });
}

void shim::add_malloc_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"malloc", ::malloc},
        {"free", ::free},
        {"calloc", ::calloc},
        {"realloc", ::realloc},
        {"valloc", ::valloc},
        {"posix_memalign", ::posix_memalign},
        {"_Znwj", (void *(*)(size_t)) ::operator new},
        {"_ZdlPv", (void (*)(void *)) ::operator delete},
    });
}

void shim::add_ctype_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"_tolower_tab_", &_tolower_tab_},
        {"_toupper_tab_", &_toupper_tab_},
        {"_ctype_", &_ctype_},
        {"isalnum", isalnum},
        {"isalpha", isalpha},
        {"isblank", isblank},
        {"iscntrl", iscntrl},
        {"isdigit", isdigit},
        {"isgraph", isgraph},
        {"islower", islower},
        {"isprint", isprint},
        {"ispunct", ispunct},
        {"isspace", isspace},
        {"isupper", isupper},
        {"isxdigit", isxdigit},

        {"tolower", ::tolower},
        {"toupper", ::toupper},
    });
}

void shim::add_math_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"__umoddi3", __umoddi3},
        {"__udivdi3", __udivdi3},
        {"__divdi3", __divdi3},
        {"ldexp", (double (*)(double, int)) ::ldexp},
    });
}

void shim::add_sched_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"sched_yield", ::sched_yield},
    });
}

std::vector<shimmed_symbol> shim::get_shimmed_symbols() {
    std::vector<shimmed_symbol> ret;
    add_common_shimmed_symbols(ret);
    add_stdlib_shimmed_symbols(ret);
    add_malloc_shimmed_symbols(ret);
    add_ctype_shimmed_symbols(ret);
    add_math_shimmed_symbols(ret);
    add_sched_shimmed_symbols(ret);
    add_pthread_shimmed_symbols(ret);
    add_sem_shimmed_symbols(ret);
    add_network_shimmed_symbols(ret);
    add_dirent_shimmed_symbols(ret);
    add_cstdio_shimmed_symbols(ret);
    return ret;
}