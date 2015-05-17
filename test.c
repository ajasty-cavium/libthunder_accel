#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFLEN (4096*1024)

extern void accel_announce(void);
extern void *memcpy_d(void *, const void *, size_t);

char destbuf[BUFLEN], srcbuf[BUFLEN];

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
  //fprintf(stderr, "starting.\n");
  
  memcpy(dest, src, BUFLEN);
  //printf("memcpy %i\n", BUFLEN);
}

int cmpbuf(const char *src, const char *dest)
{
    return memcmp(src, dest, BUFLEN);
}

int main(int argc, char **argv)
{
  //xaccel_announce();
  int i;
  for (i = 0; i < 2000; i++) {
  clrbuf(destbuf);
    setbufv(srcbuf, 1);
    cpybuf(srcbuf, destbuf);
    if(cmpbuf(srcbuf, destbuf)) {
	printf("ERROR: copy failed.\n");
    }
  }
    //printf("copy succeeded.\n");
}

