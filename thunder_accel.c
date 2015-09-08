#define _GNU_SOURCE
#define _HAVE_STRING_ARCH_strcmp
#define _HAVE_STRING_ARCH_strlen

#include <stdio.h>
#include <string.h>

#define HANDLE_BZERO  0
#define HANDLE_MEMSET 0
#define HANDLE_MEMCPY 1
#define HANDLE_MEMCMP 0
#define HANDLE_STRCPY 0
#define HANDLE_STRCMP 0
#define HANDLE_STRLEN 0

void *(*memcpy_c)(void *, const void *, size_t);
int (*memcmp_c)(const void *, const const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
int memcmp_s(const void *s1, const void *s2, size_t len);
void *memset_s(void *s, int c, size_t n);
void bzero_s(void *s, size_t n);

char *strcpy_s(char *dest, const char *src);
int strcmp_s(const char *s1, const char *s2);
size_t strlen_s(const char *s);

extern void accel_announce(void);

#if HANDLE_MEMCPY
void *memcpy(void *dest, const void *src, size_t len)
{
    return memcpy_s(dest, src, len);
}
#endif

#if HANDLE_MEMCMP
int memcmp(const void *s1, const void *s2, size_t len)
{
    return memcmp_s(s1, s2, len);
}
#endif

#if HANDLE_MEMSET
void *memset(void *s, int c, size_t n)
{
    return memset_s(s, c, n);
}
#endif

#if HANDLE_BZERO
void bzero(void *s, size_t n)
{
    bzero_s(s, n);
}
#endif

#if HANDLE_STRCPY
char *strcpy(char *dest, const char *src)
{
    return strcpy_s(dest, src);
}
#endif

#if HANDLE_STRCMP
int strcmp(const char *s1, const char *s2)
{
    return strcmp_x(s1, s2);
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
