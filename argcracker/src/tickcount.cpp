#if defined(MACOS)
#include <sys/sysctl.h>
#include <mach/mach_time.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

#if defined(MACOS)
// from XIV-on-Mac, I think it changed in recent Wine versions?
uint32_t TickCount() {
    struct mach_timebase_info timebase;
    mach_timebase_info(&timebase);

    auto machtime = mach_continuous_time();
    auto numer = uint64_t (timebase.numer);
    auto denom = uint64_t(timebase.denom);
    auto monotonic_time = machtime * numer / denom / 100;
    return monotonic_time / 10000;
}
#endif

#if defined(LINUX)
uint32_t TickCount() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if defined(WIN32)
uint32_t TickCount() {
    return GetTickCount();
}
#endif