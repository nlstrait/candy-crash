# define the commands we will use for compilation
CXX = g++

# where the socket source code lives
SOCKDIR = ../socketCode

# Two-dimensional array library
LIBS=../../array2d/libhw2.a  

# define useful flags to cc/ld/etc.
CFLAGS = -g -Wall -Wpedantic -O0 -I$(SOCKDIR)
IFLAGS =  -I../../array2d -I../../jansson/include
GTKCFLAGS = `pkg-config --cflags gtk+-3.0`
LDFLAGS =  -L../../array2d -L../../jansson/lib -lhw2 -l:libjansson.a
GTKLDFLAGS = `pkg-config --libs gtk+-3.0`

# define common dependencies
HEADERS = $(SOCKDIR)/CSE333.h $(SOCKDIR)/CSE333Socket.h view.h model.h Candy.h GameState.h GameDef.h Adapter.h Block.h
OBJS = $(SOCKDIR)/CSE333Socket.o $(SOCKDIR)/Verify333.o $(SOCKDIR)/ClientSocket.o Candy.o GameState.o GameDef.o Block.o

all: model view

view: view.o $(OBJS) $(SOCKDIR)/ServerSocket.o Adapter.o
	cd $(SOCKDIR) && make
	$(CXX) $(CFLAGS) -o view $(SOCKDIR)/ServerSocket.o view.o Adapter.o $(OBJS) $(LDFLAGS) $(GTKLDFLAGS)

model: model.o view.o $(OBJS) $(SOCKDIR)/ServerSocket.o
	cd $(SOCKDIR) && make
	$(CXX) $(CFLAGS) -o model $(SOCKDIR)/ServerSocket.o model.o $(OBJS) $(LDFLAGS) $(GTKLDFLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) -std=c++11 $(CFLAGS) -c $(IFLAGS) $(GTKCFLAGS) $<

clean: FORCE
	/bin/rm -f *.o *~ model view
	/bin/rm -f $(SOCKDIR)/*.o

FORCE:
