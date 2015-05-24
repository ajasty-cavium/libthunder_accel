#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
//typedef unsigned long size_t;

#define MEMCPY_FALLBACK 0
#define MEMCMP_FALLBACK 0

void *(*memcpy_c)(void *, const void *, size_t);
int (*memcmp_c)(const void *, const const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
extern void *memcpy_rep(void *dest, const void *src, size_t len);
extern void *memcpy_repe(void *dest, const void *src, size_t len);
int memcmp_s(const void *s1, const void *s2, size_t len);



extern void accel_announce(void);

void *memcpy(void *dest, const void *src, size_t len)
{
#if MEMCPY_FALLBACK
  /*printf("memcpy_s %i.\n", len);*/
  if (len < (1<<15)) {

  if (!memcpy_c) {
    memcpy_c = dlsym(RTLD_NEXT, "memcpy");
  }

  return memcpy_c(dest, src, len);
  }
#endif
#if defined(AARCH64)
  return memcpy_repe(dest, src, len);
#elif defined(X64)
  return memcpy_repe(dest, src, len);
#endif
}

int memcmp(const void *s1, const void *s2, size_t len)
{
 #if MEMCMP_FALLBACK
  /*printf("memcpy_s %i.\n", len);*/
  if (len < (1<<15)) {

  if (!memcmp_c) {
    memcmp_c = dlsym(RTLD_NEXT, "memcmp");
  }
 return memcmp_c(s1, s2, len);

  }
#endif
 return memcmp_s(s1, s2, len);
}

void accel_announce()
{
  fprintf(stderr, "Initializing libthunder_accel.\n");
}
