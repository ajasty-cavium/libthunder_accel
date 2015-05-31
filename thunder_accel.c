#define _GNU_SOURCE
#define _HAVE_STRING_ARCH_strcmp

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
//typedef unsigned long size_t;

#define MEMCPY_FALLBACK 0
#define MEMCMP_FALLBACK 0

void *(*memcpy_c)(void *, const void *, size_t);
int (*memcmp_c)(const void *, const const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
int memcmp_s(const void *s1, const void *s2, size_t len);

char *strcpy_s(char *dest, const char *src);
int strcmp_s(const char *s1, const char *s2);
size_t strlen(const char *s);

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
  return memcpy_s(dest, src, len);
#elif defined(X64)
  return memcpy_s(dest, src, len);
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

char *strcpy(char *dest, const char *src)
{
  return strcpy_s(dest, src);
}

int strcmp(const char *s1, const char *s2)
{
  return strcmp_s(s1, s2);
}

size_t strlen(const char *s)
{
    return strlen_s(s);
}

void accel_announce()
{
  fprintf(stderr, "Initializing libthunder_accel.\n");
}
