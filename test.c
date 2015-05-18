#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFLEN (4096<<4)
#define NRUNS (200000)

#define MEMCLR 0
#define MEMSET 0
#define MEMCPY 1
#define MEMCMP 0

extern void accel_announce(void);
extern void *memcpy_d(void *, const void *, size_t);

char destbuf[BUFLEN], srcbuf[BUFLEN];

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

void clrbuf(char *t)
{
    setbufv(t, 0);
}

void cpybuf(const char *src, char *dest)
{
  memcpy(dest, src, BUFLEN);
}

int cmpbuf(const char *src, const char *dest)
{
    return memcmp(src, dest, BUFLEN);
}

int main(int argc, char **argv)
{
  int i;
  unsigned long long tv = get_ticks(), total = 0;
  for (i = 0; i < NRUNS; i++) {
#if MEMCLR
  clrbuf(destbuf);
#endif
#if MEMSET
    setbufv(srcbuf, 1);
#endif
#if MEMCPY
    tv = get_ticks();
    cpybuf(srcbuf, destbuf);
    total += ticks_since(tv);
#endif
#if MEMCMP
    if(cmpbuf(srcbuf, destbuf)) {
	printf("ERROR: copy failed %hhx %hhx.\n", destbuf[0], srcbuf[0]);
    }
#endif

  }
  printf("ticks total: %llu.\n", total);
}

