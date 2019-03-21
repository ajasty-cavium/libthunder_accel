#define _GNU_SOURCE
#define _HAVE_STRING_ARCH_strcmp
#define _HAVE_STRING_ARCH_strcpy

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFLEN (1<<16)
#define NRUNS (1<<4)
#define CACHECLR (1<<26)

#ifndef TEST_BZERO
#define TEST_BZERO 0
#endif
#ifndef TEST_MEMSET
#define TEST_MEMSET 0
#endif
#ifndef TEST_MEMCPY
#define TEST_MEMCPY 1
#endif
#ifndef TEST_MEMCMP
#define TEST_MEMCMP 0
#endif
#ifndef TEST_STRCPY
#define TEST_STRCPY 0
#endif
#ifndef TEST_STRCMP
#define TEST_STRCMP 0
#endif
#ifndef TEST_STRLEN
#define TEST_STRLEN 0
#endif

#ifndef TEST_ISO_CONV
#define TEST_ISO_CONV 0
#endif

extern void accel_announce(void);
extern void *memcpy_d(void *, const void *, size_t);
extern int iso_conv_s(const unsigned short *, char *, int);

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

void clearcache()
{
    memset(clrbuf, 0, CACHECLR);
}

int runtest(int len, int runs, int doprint)
{
  int i;
  unsigned long long tv;
  unsigned long long memcpy_total = 0, memcmp_total = 0;
  unsigned long long strcpy_total = 0, strcmp_total = 0;
  unsigned long long strlen_total = 0, iso_conv_total = 0;
  unsigned long long bzero_total = 0, memset_total = 0;


  for (i = 0; i < runs; i++) {
    clearcache();
#if TEST_BZERO
    tv = get_ticks();
    bzero(destbuf, len);
    bzero_total += ticks_since(tv);
    clearcache();
#endif
#if TEST_MEMSET
    tv = get_ticks();
    memset(srcbuf, 1, len);
    memset_total += ticks_since(tv);
    clearcache();
#endif
#if TEST_MEMCPY
    //len --;
    memset(destbuf, 0x55, len << 2);
    char *d = srcbuf;
    int i, j;
    for(i = 0; i < (len >> 0); i+=16) {
	for (j = 0; j < ( 16); j++) {
	    *(d++) = j;
	}
    }
    //memset(srcbuf, 0xaa, len << 2);
    clearcache();
    tv = get_ticks();
    memcpy(destbuf + len, srcbuf, len);
    memcpy_total += ticks_since(tv);
    if (runs > 1) {
    printf("%p-%08i:  %08lx: %016lx %08lx: %016lx.\n", destbuf, len, (unsigned long) (destbuf + len - 4), *((unsigned long long*)(destbuf + len - 4)), (unsigned long) (destbuf + (len << 1) - 4), *((unsigned long long*)(destbuf + (len << 1) - 4)));
    } else {
	unsigned long *dt = (destbuf + len - 4);
	//char *s = srcbuf;
	for (i = 0; i <= len; i += 16) {
	    printf("%p: %016lx %016lx\n", dt, *(dt++), *(dt++));
	}
    }
    clearcache();
#endif

#if TEST_MEMCMP
    tv = get_ticks();
    if(memcmp(srcbuf, destbuf, len)) {
	printf("ERROR: memcpy failed %hhx %hhx.\n", destbuf[0], srcbuf[0]);
    }
    memcmp_total += ticks_since(tv);
    clearcache();
#endif

#if TEST_STRCPY
    memset(srcbuf, ' ', len);
    destbuf[len - 1] = srcbuf[len - 1] = '\0';
    tv = get_ticks();
    strcpy(destbuf, srcbuf);
    strcpy_total += ticks_since(tv);
    clearcache();
#endif

#if TEST_STRCMP
    tv = get_ticks();
    if (strcmp(destbuf, srcbuf)) {
	printf("ERROR: strcpy failed %hhx %hhx.\n", destbuf[0], srcbuf[0]);
    }
    strcmp_total += ticks_since(tv);
    clearcache();
#endif

#if TEST_STRLEN
    {
	int slen;
        tv = get_ticks();
	slen = strlen(destbuf);
        strlen_total += ticks_since(tv);
	if (slen != (len - 1))
	    printf("strlen = %i not %i.\n", slen, len - 1);
    }
    clearcache();
#endif

#if TEST_ISO_CONV
    {
	int c = 32, i = 1000;
	swprintf((unsigned short*)srcbuf, "%s", L"aaaaaaaabbbbbbbbccccccccdddddddd.");
	tv = get_ticks();
	while(i--)
		iso_conv_s((const unsigned short*)srcbuf, (char*)destbuf, c);
	iso_conv_total += ticks_since(tv);
	//printf("iso_conv = %i.\n", c);
    }
    {
	int c = 33;
	memset(destbuf, 0, 66);
	//c = iso_conv_s((const unsigned short*)srcbuf, (char*)destbuf, c);
	//c = swprintf((unsigned short*)srcbuf, "%s", L"aaaaaaaabbbbbbbbccccccccdddddddd.");
	wcscpy(srcbuf, L"aaaaaaaabbbbbbbbccccccccdddddddd.");
	printf("len=%i %ls.\n", c, srcbuf);
	c = iso_conv_s((const unsigned short*) srcbuf, (char*)destbuf, c);
	printf("iso_conv = %i %s.\n", c, destbuf);
	write(1, destbuf, c);
    }

#endif
    

  }
  if (!doprint) return 0;

 fprintf(stderr, "%i runs, ticks total:\n", runs); 
#if TEST_BZERO
  printf("\tbzero:\t%llu\n", bzero_total / runs);
#endif
#if TEST_MEMSET
  printf("\tmemset:\t%llu\n", memset_total / runs);
#endif
#if TEST_MEMCPY
  fprintf(stdout, "\tmemcpy:\t%llu\n", memcpy_total / runs);
#endif
#if TEST_MEMCMP
  printf("\tmemcmp:\t%llu\n", memcmp_total / runs);
#endif
#if TEST_STRCPY
  printf("\tstrcpy:\t%llu\n", strcpy_total / runs);
#endif
#if TEST_STRCMP
  printf("\tstrcmp:\t%llu\n", strcmp_total / runs);
#endif
#if TEST_STRLEN
  printf("\tstrlen:\t%llu\n", strlen_total / runs);
#endif
#if TEST_ISO_CONV
  printf("\tiso_conv:\t%llu\n", iso_conv_total / runs);
#endif
  return 0;
}

int main(int argc, void **argv)
{
  int v = BUFLEN, r = NRUNS;

  if (argc >= 2) v = atoi(argv[1]);
  if (argc >= 3) r = atoi(argv[2]);

  srcbuf = malloc(v << 2);
  destbuf = malloc(v << 2);
  clrbuf = malloc(CACHECLR);

  //runtest(1, 1, 0);

  return runtest(v, r, 1);
}

