#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
//typedef unsigned long size_t;

void *(*memcpy_c)(void *, const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
extern void *memcpy_rep(void *dest, const void *src, size_t len);
extern void *memcpy_repe(void *dest, const void *src, size_t len);
extern void accel_announce(void);

void *memcpy(void *dest, const void *src, size_t len)
{
  /*printf("memcpy_s %i.\n", len);*/
#if 0
  if (!memcpy_c) {
    memcpy_c = dlsym(RTLD_NEXT, "memcpy");
  }

  return memcpy_c(dest, src, len);
#else
#if defined(AARCH64)
#elif defined(X64)
  return memcpy_repe(dest, src, len);
#endif
#endif
}

void accel_announce()
{
  fprintf(stderr, "Initializing libthunder_accel.\n");
}
