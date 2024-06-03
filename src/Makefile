CC=gcc
C++=g++
LD=g++
CFLAGS=-c -Wall -pedantic-errors -O0 -g3
LDFLAGS=-pthread
OBJECTS=./includes/fileutil.o ./includes/parser.o ./includes/send.o ./includes/session.o ./includes/socketutil.o
OBJECT1=shserver.o
OBJECT2=shserver_sl.o
OBJECT3=shserver_th.o
OBJECT4=shserver_mp.o
EXECUTABLE1=shserver.out
EXECUTABLE2=shserver_sl.out
EXECUTABLE3=shserver_th.out
EXECUTABLE4=shserver_mp.out

all: $(EXECUTABLE1) $(EXECUTABLE2) $(EXECUTABLE3) $(EXECUTABLE4)

$(EXECUTABLE1): $(OBJECT1) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECT1) $(OBJECTS) -o $@

$(EXECUTABLE2): $(OBJECT2) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECT2) $(OBJECTS) -o $@

$(EXECUTABLE3): $(OBJECT3) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECT3) $(OBJECTS) -o $@

$(EXECUTABLE4): $(OBJECT4) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECT4) $(OBJECTS) -o $@

.cpp.o:
	$(C++) $(CFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f ${EXECUTABLE} ${EXECUTABLE}.exe ${OBJECTS} ${OBJECT1} ${OBJECT2} ${OBJECT3} ${OBJECT4}

