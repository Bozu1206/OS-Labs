#ifdef __cplusplus
extern "C" {
#endif

static inline __attribute__ ((always_inline))
void magma_log(int bug, int condition)
{
    if (condition) {
        extern int printf(const char *format, ...);
        printf("[MAGMA] Bug %d triggered!\n", bug);
    }

#ifdef MAGMA_FATAL_CANARIES
#ifdef __cplusplus
    #define __THROW throw()
#else
    #define __THROW
#endif

    extern int getpid(void) __THROW;
    extern int kill(int, int) __THROW;

    // send SIGSEGV to self
    kill(getpid(), (condition)*11);
#endif
}

#ifdef __cplusplus
}
#endif
