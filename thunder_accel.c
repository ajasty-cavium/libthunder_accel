#define _GNU_SOURCE
#define _HAVE_STRING_ARCH_strcmp
#define _HAVE_STRING_ARCH_strlen

#include <stdio.h>
#include <string.h>

#define HANDLE_STRCPY 1
#define HANDLE_STRCMP 1
#define HANDLE_STRLEN 1

void *(*memcpy_c)(void *, const void *, size_t);
int (*memcmp_c)(const void *, const const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
int memcmp_s(const void *s1, const void *s2, size_t len);

char *strcpy_s(char *dest, const char *src);
int strcmp_s(const char *s1, const char *s2);
size_t strlen_s(const char *s);

extern void accel_announce(void);

void *memcpy(void *dest, const void *src, size_t len)
{
    return memcpy_s(dest, src, len);
}

int memcmp(const void *s1, const void *s2, size_t len)
{
    return memcmp_s(s1, s2, len);
}

#if HANDLE_STRCPY
char *strcpy(char *dest, const char *src)
{
    return strcpy_s(dest, src);
}
#endif

#if HANDLE_STRCMP
int strcmp(const char *s1, const char *s2)
{
    return strcmp_s(s1, s2);
}
#endif

#if HANDLE_STRLEN
size_t strlen(const char *s)
{
    return strlen_s(s);
}
#endif

void accel_announce()
{
  fprintf(stderr, "Initializing libthunder_accel.\n");
}
