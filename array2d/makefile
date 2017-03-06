# define the commands we will use for compilation and library building
AR = ar
ARFLAGS = rcs
CC = gcc

# define useful flags to cc/ld/etc.
#CFLAGS += -std=c11 -g -Wall -Wpedantic -I. -I../jansson/include -O0
CFLAGS += -std=c11 -O3 -Wall -Wpedantic -I. -I../jansson/include -O0
LDFLAGS += -L. -L../jansson/lib -lhw2 -l:libjansson.a

TESTSRC=test.c
LIBOBJS=array2d.o
HEADERS=array2d.h

all: libhw2.a 

libhw2.a: $(LIBOBJS) $(HEADERS)
	$(AR) $(ARFLAGS) libhw2.a $(LIBOBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	/bin/rm -f *.o *~ libhw2.a


