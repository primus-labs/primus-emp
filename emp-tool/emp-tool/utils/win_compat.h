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
#include <cstdio>
#include <mutex>
#include <io.h>       // _open_osfhandle, _close
#include <fcntl.h>    // _O_RDWR, _O_BINARY

// POSIX usleep() shim (argument is microseconds). Windows Sleep() is ms-granular.
#ifndef usleep
static inline void emp_usleep(uint64_t usec) { Sleep((DWORD)((usec + 999) / 1000)); }
#define usleep(us) emp_usleep((uint64_t)(us))
#endif

// POSIX fmemopen() shim. MinGW-w64's CRT has no fmemopen, so BristolFormat's
// "load circuit from an in-memory string" ctor won't compile/link on Windows.
// Back it with a Win32 temp file opened FILE_FLAG_DELETE_ON_CLOSE: the file is
// removed automatically when the handle closes (or at process exit), so nothing
// is left on disk. Binary mode is used deliberately — fmemopen does no newline
// translation either, and the Bristol parser reads with fscanf on whitespace.
#ifndef fmemopen
static inline FILE* emp_fmemopen(const void* buf, size_t size, const char* mode) {
    (void)mode;
    char dir[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, dir);
    if (n == 0 || n > MAX_PATH) return NULL;
    char path[MAX_PATH];
    if (GetTempFileNameA(dir, "emp", 0, path) == 0) return NULL;
    HANDLE h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                           NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    int fd = _open_osfhandle((intptr_t)h, _O_RDWR | _O_BINARY);
    if (fd == -1) { CloseHandle(h); return NULL; }
    FILE* f = _fdopen(fd, "wb+");
    if (!f) { _close(fd); return NULL; }
    if (size) fwrite(buf, 1, size, f);
    fflush(f);
    fseek(f, 0, SEEK_SET);
    return f;
}
#define fmemopen(buf, size, mode) emp_fmemopen((buf), (size), (mode))
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
