SRC=view.cc model.cc Candy.cc GameState.c GameDef.c Adapter.cc Block.cc
HEADERS=view.h model.h Candy.h GameState.h GameDef.h Adapter.h Block.h
LIBS=../array2d/libhw2.a
CFLAGS = -Wall -Wpedantic -std=c++11 
IFLAGS =  -I../array2d -I../jansson/include
GTKCFLAGS = `pkg-config --cflags gtk+-3.0`
LDFLAGS =  -L../array2d -L../jansson/lib -lhw2 -l:libjansson.a
GTKLDFLAGS = `pkg-config --libs gtk+-3.0`

all: game

game: $(SRC) $(HEADERS) $(LIBS)
	g++ $(CFLAGS) -g -O0 $(IFLAGS) $(GTKCFLAGS) -o game $(SRC) $(LDFLAGS) $(GTKLDFLAGS)

clean:
	rm -f *.o *~ game gameOut.json
