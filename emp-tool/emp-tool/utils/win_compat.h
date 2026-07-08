#ifndef EMP_UTIL_WIN_COMPAT_H__
#define EMP_UTIL_WIN_COMPAT_H__

// Windows (MinGW-w64) portability shim for emp-toolkit.
//
// MinGW already provides <unistd.h> and <pthread.h> (winpthread), so only the
// BSD-socket headers and a couple of POSIX helpers are missing on Windows.
// This header centralizes those differences so the network I/O channels COMPILE
// under MinGW.
//
// SCOPE (P1): compile-only. emp's own NetIO/HighSpeedNetIO socket *runtime* path
// is intentionally NOT ported here — it still uses close() (not closesocket),
// never calls WSAStartup, and does not translate Winsock error codes. That is
// deferred to P2. In pado, real traffic goes through pado's own proxy channel,
// so emp's NetIO is only used as a template type and is not exercised at runtime.
// emp_wsa_init() below is the hook P2 will wire into the NetIO constructor.

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// The NetIO/HighSpeedNetIO inline method bodies name ssize_t and close(); their
// declarations must be visible even though the socket runtime is not exercised in
// P1 (pado uses its own proxy channel). MinGW-w64 provides these headers.
// NOTE (P1 compile-only): close() here is the CRT file-descriptor close(), NOT a
// socket close — real socket cleanup (closesocket) is deferred to P2.
#include <sys/types.h>   // ssize_t
#include <unistd.h>      // close()

#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif

#include <cstdint>
#include <mutex>

// POSIX usleep() shim (argument is microseconds). Windows Sleep() is ms-granular.
#ifndef usleep
static inline void emp_usleep(uint64_t usec) { Sleep((DWORD)((usec + 999) / 1000)); }
#define usleep(us) emp_usleep((uint64_t)(us))
#endif

// One-time Winsock initialization; safe to call repeatedly.
static inline void emp_wsa_init() {
    static std::once_flag once;
    std::call_once(once, []() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    });
}

#else // !_WIN32 (POSIX)

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

static inline void emp_wsa_init() {}

#endif // _WIN32

#endif // EMP_UTIL_WIN_COMPAT_H__
