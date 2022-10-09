# Copyright 2022 Mark Seminatore. All rights reserved.

TARGET	= asm
LINKER	= cc -o

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
LIBS = -lm

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $(CFLAGS) $<

$(TARGET):	$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)
	

