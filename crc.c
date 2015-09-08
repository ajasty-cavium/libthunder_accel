
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __GNUC__
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

#define LDP(x,y,p) asm("ldp %x[a], %x[b], [%x[c]], #16" : [a]"=r"(x),[b]"=r"(y),[c]"+r"(p))

#define CRC32CX(crc,value) asm("crc32cx %w[c], %w[c], %x[v]" : [c]"+r"(*&crc) : [v]"r"(+value))
#define CRC32CW(crc,value) asm("crc32cw %w[c], %w[c], %w[v]" : [c]"+r"(*&crc) : [v]"r"(+value))
#define CRC32CH(crc,value) asm("crc32ch %w[c], %w[c], %w[v]" : [c]"+r"(*&crc) : [v]"r"(+value))
#define CRC32CB(crc,value) asm("crc32cb %w[c], %w[c], %w[v]" : [c]"+r"(*&crc) : [v]"r"(+value))

#define CRC32ZX(crc,value) asm("crc32x %w[c], %w[c], %x[v]" : [c]"+r"(crc) : [v]"r"(value))
#define CRC32ZW(crc,value) asm("crc32w %w[c], %w[c], %w[v]" : [c]"+r"(crc) : [v]"r"(value))
#define CRC32ZH(crc,value) asm("crc32h %w[c], %w[c], %w[v]" : [c]"+r"(crc) : [v]"r"(value))
#define CRC32ZB(crc,value) asm("crc32b %w[c], %w[c], %w[v]" : [c]"+r"(crc) : [v]"r"(value))

#ifdef DEBUG
#define pr(...) printf(__VA_ARGS__)
#else
#define pr(...) {}
#endif

#include "crc32c.h"

uint64_t gettsc()
{
    uint64_t cval;
    asm volatile("mrs %0, cntvct_el0" : "=r" (cval));
    return cval;
}

void blockcrc(const uint8_t *p_buf1, uint32_t *crc1, int len)
{
    uint64_t x1, y1;
    uint64_t c1 = *crc1;

    pr("len %i.\n", len);
    asm (".cpu generic+crc\n");
    if (len >= 16) {
	if (unlikely((((uint64_t)p_buf1)& 1))) {
	    x1 = *p_buf1;
	    CRC32CB(c1, x1);
	    p_buf1++;
	    len--;
	    pr("1 ");
	}
	if (unlikely((((uintptr_t)p_buf1)& sizeof(uint16_t) ))) {
	    x1 = *(uint16_t*)p_buf1;
	    CRC32CH(c1, x1);
	    p_buf1+=sizeof(uint16_t);
	    len-=sizeof(uint16_t);
	    pr("2 ");
	}
	if (unlikely((((uintptr_t)p_buf1)& sizeof(uint32_t) ))) {
	    x1 = *(uint32_t*)p_buf1;
            CRC32CW(c1, x1);
	    p_buf1+=sizeof(uint32_t);
	    len-=sizeof(uint32_t);
	    pr("3 ");
	}
	if (unlikely((((uintptr_t)p_buf1)& sizeof(uint64_t) ))) {
	    x1 = *(uint64_t*)p_buf1;
            CRC32CX(c1, x1);
	    p_buf1+=sizeof(uint64_t);
	    len-=sizeof(uint64_t);
	    pr("4 ");
	}

        pr("len %i.\n", len);
        /* single block */
	while ((len -= 2*sizeof(uint64_t)) >= 0) {
	    LDP(x1,y1,p_buf1);
	    CRC32CX(c1, x1);
	    CRC32CX(c1, y1);
	    pr("5 ");
	}
    }
    *crc1 = c1;
}

int main(int argc, char **argv)
{
    uint64_t s;
    uint32_t d, len = 1<<19;
    uint64_t ts = gettsc();
    uint32_t crc = 0;

    if (argc > 1) sscanf(argv[1], "%i", &len);

    s = (uint64_t) malloc(len + 32);
    s &= ~0xf;

    /* We're just doing this to preload the cache. */
    ts = gettsc();
    //blockcrc((uint8_t*)s, &d, len);

    ts = gettsc();
    crc = crc32c_block(crc, (const void*)s, len);
    printf("took %li cycles %x.\n", gettsc() - ts, crc);
    ts = gettsc();
    crc = crc32c_block(crc, (const void*)s, len);
    printf("took %li cycles, %x.\n", gettsc() - ts, crc);
    ts = gettsc();
    crc = crc32c_block(crc, (const void*)s, len);
    printf("took %li cycles %i/cl %x.\n", gettsc() - ts, (gettsc()-ts) / (len >> 7), crc);


    return 0;
}
