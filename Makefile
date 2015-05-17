
ARCH=AARCH64

CC=gcc
AR=ar
RANLIB=ranlib

ifeq ($(ARCH),X64)
	CDEFS=-DX64
	MEMCPY_O=memcpy_64.o
	MEMCPY_S=memcpy_64.S
else
ifeq ($(ARCH),AARCH64)
	CDEFS=-DAARCH64
	MEMCPY_O=memcpy_arm64.o
	MEMCPY_S=memcpy_arm64.S
endif
endif

CFLAGS=-fPIC -ggdb -O0 $(CDEFS)
LDFLAGS=-shared  -fPIC -ggdb -O0 -rdynamic 

LIBTHUNDER_OBJS = thunder_accel.o $(MEMCPY_O)
LIBTHUNDER_LOBJS = memcpy_64.lo thunder_accel.lo
LIBTHUNDER_SRCS = thunder_accel.c $(MEMCPY_S)

all: libthunder_accel.so test

test: test.o
	$(CC) $(CFLAGS) -o $@ $<

test.o: test.c
	$(CC) $(CFLAGS) -c $< -o $@

libthunder_accel.so: libthunder_accel.a
	$(CC) $(LDFLAGS) -o libthunder_accel.so $(LIBTHUNDER_OBJS) -ldl

libthunder_accel.a: $(LIBTHUNDER_OBJS)
	$(AR) cru libthunder_accel.a $(LIBTHUNDER_OBJS)
	$(RANLIB) libthunder_accel.a

thunder_accel.o: thunder_accel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MEMCPY_O): $(MEMCPY_S)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIBTHUNDER_OBJS) libthunder_accel.so libthunder_accel.a test test.o
	rm -rf .libs

run: all
	LD_PRELOAD=/home/bill/src/libthunder_accel/libthunder_accel.so ./test
