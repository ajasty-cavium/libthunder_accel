
#define CRC_INST() asm(".cpu generic+crc");

#define PRFM(x,y) asm("prfm pldl1strm, [%x[a], "  y  "]" : : [a]"r"(x))
#define LDP(x,y,p) asm("ldp %x[a], %x[b], [%x[c]], #16" : [a]"=r"(x),[b]"=r"(y),[c]"+r"(p))
#define CRC32CX(crc,value) asm("crc32cx %w[c], %w[c], %x[v]" : [c]"+r"(*&crc) : [v]"r"(+value))
#define CRC32CW(crc,value) asm("crc32cw %w[c], %w[c], %w[v]" : [c]"+r"(*&crc) : [v]"r"(+value))
#define CRC32CB(crc,value) asm("crc32cb %w[c], %w[c], %w[v]" : [c]"+r"(*&crc) : [v]"r"(+value))

#define CRC32ZX(crc,value) asm("crc32x %w[c], %w[c], %x[v]" : [c]"+r"(crc) : [v]"r"(value))
#define CRC32ZB(crc,value) asm("crc32b %w[c], %w[c], %w[v]" : [c]"+r"(crc) : [v]"r"(value))

int crc32c_has_wide_crc()
{
    return 1;
}

inline uint32_t crc32c_128(uint32_t crc, const void *data) __attribute__((__always_inline__));
inline uint32_t crc32c_128(uint32_t crc, const void *data)
{
    CRC_INST();
    uint64_t s0, s1, s2, s3, s4, s5;
    	PRFM(data, "384");
	LDP(s0, s1, data);
	LDP(s2, s3, data);
	CRC32CX(crc, s0);
	CRC32CX(crc, s1);
	LDP(s4, s5, data);
	CRC32CX(crc, s2);
	CRC32CX(crc, s3);
	LDP(s0, s1, data);
	CRC32CX(crc, s4);
	CRC32CX(crc, s5);
	LDP(s2, s3, data);
	CRC32CX(crc, s0);
	CRC32CX(crc, s1);
	LDP(s4, s5, data);
	CRC32CX(crc, s2);
	CRC32CX(crc, s3);
	LDP(s0, s1, data);
	CRC32CX(crc, s4);
	CRC32CX(crc, s5);
	LDP(s2, s3, data);
	CRC32CX(crc, s0);
	CRC32CX(crc, s1);
	CRC32CX(crc, s2);
	CRC32CX(crc, s3); 

	return crc;
}

inline uint32_t crc32c_block(uint32_t crc, const void *data, int len)
{
    uint64_t s0, s1, s2, s3, s4, s5;
    uint8_t *c;
    uint32_t *d;

    CRC_INST();
    if (crc32c_has_wide_crc()) {
	while (len > 128) {
	   crc = crc32c_128(crc, data);
	   data += 128;
	    len -= 128;
	}
    }

    d = (uint32_t*) data;
    while (len > 4) {
	CRC32CW(crc, *(d++));
	len -= 4;
    }

    c = (uint8_t*) data;
    while (len--) {
	CRC32CB(crc, *(c++));
    }
    return crc;
}

