# Copyright 2022 Mark Seminatore. All rights reserved.

TARGET	= asm
LINKER	= cpp -o

DEPS 	= \
	../aout.h  \
	../cpu_cisc.h

OBJS	= \
	main.o \
	../aout.o \
	ParserKit/baseparser.o \
	ParserKit/lexer.o \
	ParserKit/sumboltable.o

CFLAGS	= -I. -I.. -g -std=c++14
LIBS = -lm -lc++

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)
	

