// Included into every file by the Makefile (-include): glibc 2.27/2.29 added new versions of
// powf and pow, which would make the binary need that glibc. The old versions do the same job.
#if defined(__linux__) && defined(__aarch64__)
__asm__(".symver pow,pow@GLIBC_2.17");
__asm__(".symver powf,powf@GLIBC_2.17");
#endif
