#define _GNU_SOURCE
#define _HAVE_STRING_ARCH_strcmp
#define _HAVE_STRING_ARCH_strlen

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#define HANDLE_BZERO  0
#define HANDLE_MEMSET 0
#define HANDLE_MEMCPY 1
#define HANDLE_MEMCMP 0
#define HANDLE_STRCPY 0
#define HANDLE_STRCMP 0
#define HANDLE_STRLEN 0
#define HANDLE_ISO_CONV 0
#define HANDLE_MUTEX_LOCK 1

void *(*memcpy_c)(void *, const void *, size_t);
int (*memcmp_c)(const void *, const const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
int memcmp_s(const void *s1, const void *s2, size_t len);
void *memset_s(void *s, int c, size_t n);
void bzero_s(void *s, size_t n);

char *strcpy_s(char *dest, const char *src);
int strcmp_s(const char *s1, const char *s2);
size_t strlen_s(const char *s);

int iso_conv_s(const unsigned short*, char*, int);

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

#if HANDLE_ISO_CONV
int iso_conv(const unsigned short *c, char *d, int l)
{
    return iso_conv_s(c, d, l);
}
#endif

#if HANDLE_MUTEX_LOCK
#define _GNU_SOURCE
#include <pthread.h>

#define MAX_USER_SPIN 3
#define MAX_YIELD_SPIN 10

int (*real_pthread_mutex_lock)(pthread_mutex_t*);
int yield_spin = -1, user_spin = -1;

static void run_once(void) __attribute__((constructor));
void run_once()
{
    char *ys = getenv("LIBTXL_YIELD_SPIN");
    char *us = getenv("LIBTXL_USER_SPIN");
    yield_spin = MAX_YIELD_SPIN;
    user_spin = MAX_USER_SPIN;
    if (ys != NULL) yield_spin = atoi(ys);
    if (us != NULL) user_spin = atoi(ys);
    real_pthread_mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	int i, j;

	for (j = 0; j < yield_spin; j++) {
		for (i = 0; i < user_spin; i++) {
		        if (pthread_mutex_trylock(mutex) == 0)
				return 0;
		}
		pthread_yield();
	}

	return (*real_pthread_mutex_lock)(mutex);
}
#endif

void accel_announce()
{
  fprintf(stderr, "Initializing libthunder_accel.\n");
}
