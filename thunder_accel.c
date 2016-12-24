#define _GNU_SOURCE
#define _HAVE_STRING_ARCH_strcmp
#define _HAVE_STRING_ARCH_strlen

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>

#define HANDLE_BZERO  0
#define HANDLE_MEMSET 0
#define HANDLE_MEMCPY 1
#define HANDLE_MEMCMP 0
#define HANDLE_STRCPY 0
#define HANDLE_STRCMP 0
#define HANDLE_STRLEN 0
#define HANDLE_ISO_CONV 0
#define HANDLE_MUTEX_LOCK 1

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

void *(*memcpy_c)(void *, const void *, size_t);
int (*memcmp_c)(const void *, const const void *, size_t);

extern void *memcpy_s(void *dest, const void *src, size_t len);
extern void *memcpy_t(void *dest, const void *src, size_t len);
int memcmp_s(const void *s1, const void *s2, size_t len);
void *memset_s(void *s, int c, size_t n);
void bzero_s(void *s, size_t n);

char *strcpy_s(char *dest, const char *src);
int strcmp_s(const char *s1, const char *s2);
size_t strlen_s(const char *s);

int iso_conv_s(const unsigned short*, char*, int);

extern void accel_announce(void);

#ifdef MEMCPY_STATS

#define NBINS 32
#define MEMCPY_ALIGNMENT 0x3f

struct alignbins {
    uint64_t aligned, unaligned_src, unaligned_dest, unaligned_both;
};

struct alignbins bins[NBINS + 1];

void sigusr2(int sig)
{
    unsigned long i;

    for (i = 0; i <= NBINS; i++)
	fprintf(stderr, "%8llu: \t%llu\t%llu\t%llu\t%llu.\n", 1ull << i, bins[i].aligned, bins[i].unaligned_src, bins[i].unaligned_dest, bins[i].unaligned_both);
}

#endif

#if HANDLE_MEMCPY
void *memcpy(void *dest, const void *src, size_t len)
{
#ifdef MEMCPY_STATS
    int bin = 63 - __builtin_clzl(len);
    if (bin > NBINS) bin = NBINS;
    if (((uint64_t)dest) & MEMCPY_ALIGNMENT) {
	if (((uint64_t)src) & MEMCPY_ALIGNMENT) {
	    __atomic_add_fetch(&bins[bin].unaligned_both, 1, __ATOMIC_RELAXED);
	} else
	    __atomic_add_fetch(&bins[bin].unaligned_dest, 1, __ATOMIC_RELAXED);
    } else {
    	if (((uint64_t)src) & MEMCPY_ALIGNMENT) {
	    __atomic_add_fetch(&bins[bin].unaligned_src, 1, __ATOMIC_RELAXED);
	} else
	    __atomic_add_fetch(&bins[bin].aligned, 1, __ATOMIC_RELAXED);
    }
#endif
    return memcpy_t(dest, src, len);
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
//#include <tls.h>

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
    if (us != NULL) user_spin = atoi(us);
    real_pthread_mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    fprintf(stderr, "mutex: yield=%i spin=%i.\n", yield_spin, user_spin);
#ifdef MEMCPY_STATS
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigusr2;
    sigaction(SIGUSR2, &sa, NULL);
#endif
    accel_announce();
}

#ifdef MEMCPY_STATS
static void run_exit(void) __attribute__((destructor));
void run_exit()
{
    sigusr2(0);
}
#endif

#if 0
typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

 
/* Data structures for mutex handling.  The structure of the attribute
   type is not exposed on purpose.  */
typedef union
{
   struct __pthread_mutex_s
   {
     int __lock;
     unsigned int __count;
     int __owner;
     unsigned int __nusers;
     int __kind;
     int __spins;
     __pthread_list_t __list;
#define __PTHREAD_MUTEX_HAVE_PREV   1
   } __data;
   char __size[__SIZEOF_PTHREAD_MUTEX_T];
   long int __align;
} pthread_mutex_t;
#endif

int internal_trylock(pthread_mutex_t *mutex)
{
    int v = 0;
    int res = __atomic_compare_exchange_n(&mutex->__data.__lock, &v, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    if (res == 0) {
	mutex->__data.__owner = 5;
	++mutex->__data.__nusers;
	return 0;
    }
    return res;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	int i, j;

	for (j = 0; j < yield_spin; j++) {
		for (i = 0; i < user_spin; i++) {
		        //if (pthread_mutex_trylock(mutex) == 0)
			if (internal_trylock(mutex) == 0)
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
