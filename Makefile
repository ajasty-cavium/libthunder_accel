
ARCH=AARCH64
#ARCH=X64

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
	MEMCMP_O=memcmp_arm64.o
	MEMCMP_S=memcmp_arm64.S

	STRCPY_O=strcpy_arm64.o
	STRCPY_S=strcpy_arm64.S
	STRCMP_O=strcmp_arm64.o
	STRCMP_S=strcmp_arm64.S
	STRLEN_O=strlen_arm64.o
	STRLEN_S=strlen_arm64.S

	BITOPS_S=bitops.S
	BITOPS_O=bitops.o

	MEMSET_S=memset_arm64.S
	MEMSET_O=memset_arm64.o

	ACCEL_O=$(MEMCPY_O) $(MEMCMP_O) $(STRCPY_O) $(STRCMP_O) $(STRLEN_O) $(MEMSET_O) $(BITOPS_O)
	ACCEL_S=$(MEMCPY_S) $(MEMCMP_S) $(STRCPY_S) $(STRCMP_S) $(STRLEN_S) $(MEMSET_S) $(BITOPS_S)
endif
endif

CFLAGS=-fPIC -ggdb -Ofast $(CDEFS) -flto
LDFLAGS=-shared  -fPIC -ggdb -Ofast -rdynamic -flto

LIBTHUNDER_OBJS = thunder_accel.o $(ACCEL_O)
LIBTHUNDER_LOBJS = memcpy_64.lo thunder_accel.lo
LIBTHUNDER_SRCS = thunder_accel.c $(ACCEL_S)

all: libthunder_accel.so test

test: test.o
	$(CC) $(CFLAGS) -o $@ $<

test.o: test.c
	$(CC) $(CFLAGS) -c $< -o $@

libthunder_accel.so: libthunder_accel.a
	$(CC) $(LDFLAGS) -o libthunder_accel.so $(LIBTHUNDER_OBJS)

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
	LD_PRELOAD=./libthunder_accel.so ./test $(BUFLEN) $(NRUNS)
	./test $(BUFLEN) $(NRUNS)
