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
	ParserKit/symboltable.o

CFLAGS	= -I. -I.. -g -std=c++14
LIBS = -lm -lc++

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

code:
	./asm -o main.o main.asm
	./asm -o rtl.o rtl.asm
	./asm -o io.o io.asm
	./asm -o string.o string.asm
	./asm -o os.o os.asm
	../ln/ln -o ../ln/a.out main.o rtl.o string.o io.o os.o

clean:
	rm -f $(OBJS) $(TARGET)
	

