#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#define BUFLEN (4096<<6)
//#define BUFLEN (4096<<4)
//#define BUFLEN (1 << 17)
//#define BUFLEN (1<<10)
#define BUFLEN (1<<9)
#define NRUNS (1<<4)
#define CACHECLR (1<<26)

#ifndef MEMCLR
#define MEMCLR 1
#endif
#ifndef MEMSET
#define MEMSET 1
#endif
#ifndef MEMCPY
#define MEMCPY 1
#endif
#ifndef MEMCMP
#define MEMCMP 1
#endif
#ifndef STRCPY
#define STRCPY 1
#endif
#ifndef STRCMP
#define STRCMP 1
#endif


extern void accel_announce(void);
extern void *memcpy_d(void *, const void *, size_t);

char *destbuf, *srcbuf, *clrbuf;

unsigned long long get_ticks(void)
{
    unsigned long long tv;
#if defined(AARCH64)
    asm volatile("mrs %0, cntvct_el0" : "=r" (tv));
#elif defined(X64)
    asm volatile ("rdtsc" : "=a"(tv));
#endif
    return tv;
}

unsigned long long ticks_since(unsigned long long tv)
{
    unsigned long long ts = get_ticks();

    return ts - tv;
}

void setbufv(char *t, int v)
{
  memset(t, v, BUFLEN);
}

void cpybuf(const char *src, char *dest)
{
  memcpy(dest, src, BUFLEN);
}

int cmpbuf(const char *src, const char *dest)
{
    return memcmp(src, dest, BUFLEN);
}

int runtest(int len)
{
  int i, runs = NRUNS;
  unsigned long long tv;
  unsigned long long memcpy_total = 0, memcmp_total = 0;
  unsigned long long strcpy_total = 0, strcmp_total = 0;
  for (i = 0; i < runs; i++) {
    memset(clrbuf, 0, CACHECLR);
#if MEMCLR
    memset(destbuf, 0, len);
    memset(clrbuf, 0, CACHECLR);
#endif
#if MEMSET
    memset(srcbuf, 1, len);
    memset(clrbuf, 0, CACHECLR);
#endif
#if MEMCPY
    tv = get_ticks();
    memcpy(srcbuf, destbuf, len);
    memcpy_total += ticks_since(tv);
    memset(clrbuf, 0, CACHECLR);
#endif

#if MEMCMP
    tv = get_ticks();
    if(memcmp(srcbuf, destbuf, len)) {
	printf("ERROR: memcpy failed %hhx %hhx.\n", destbuf[0], srcbuf[0]);
    }
    memcmp_total += ticks_since(tv);
#endif

#if STRCPY
    memset(srcbuf, ' ', len);
    destbuf[len - 1] = srcbuf[len - 1] = '\0';
    tv = get_ticks();
    strcpy(destbuf, srcbuf);
    strcpy_total += ticks_since(tv);
#endif

#if STRCMP
    tv = get_ticks();
    if (strcmp(destbuf, srcbuf)) {
	printf("ERROR: memcpy failed %hhx %hhx.\n", destbuf[0], srcbuf[0]);
    }
    strcmp_total += ticks_since(tv);
#endif

  }
  printf("ticks total:\n");
#if MEMCPY
  printf("\tmemcpy:\t%llu\n", memcpy_total);
#endif
#if MEMCMP
  printf("\tmemcmp:\t%llu\n", memcmp_total);
#endif
#if STRCPY
  printf("\tstrcpy:\t%llu\n", strcpy_total);
#endif
#if STRCMP
  printf("\tstrcmp:\t%llu\n", strcmp_total);
#endif
  return 0;
}

int main(int argc, void **argv)
{
  int v = BUFLEN;

  if (argc == 2) v = atoi(argv[1]);

  srcbuf = malloc(v);
  destbuf = malloc(v);
  clrbuf = malloc(CACHECLR);

  return runtest(v);
}

