#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#define BUFLEN (4096<<6)
//#define BUFLEN (4096<<4)
//#define BUFLEN (1 << 17)
//#define BUFLEN (1<<10)
#define BUFLEN (1<<9)
#define NRUNS (1)
#define CACHECLR (1<<26)

#define MEMCLR 1
#define MEMSET 1
#define MEMCPY 1
#define MEMCMP 1

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
  int i;
  unsigned long long tv = get_ticks(), total = 0, tv2, total2 = 0;
  //for (i = 0; i < runs; i++) {
#if MEMCLR
  memset(destbuf, 0, len);
#endif
#if MEMSET
    memset(srcbuf, 1, len);
#endif
  memset(clrbuf, 0, CACHECLR);
#if MEMCPY
    tv = get_ticks();
    memcpy(srcbuf, destbuf, len);
    total += ticks_since(tv);
#endif
  memset(clrbuf, 0, CACHECLR);

#if MEMCMP
    tv2 = get_ticks();
    if(memcmp(srcbuf, destbuf, len)) {
	printf("ERROR: copy failed %hhx %hhx.\n", destbuf[0], srcbuf[0]);
    }
    total2 += ticks_since(tv2);
#endif

 // }
  printf("ticks total: %llu %llu.\n", total, total2);
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

