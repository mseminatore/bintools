..\asm -o main.o main.asm
..\asm -o string.o string.asm
..\asm -o rtl.o rtl.asm
..\asm -o io.o io.asm
..\asm -o os.o os.asm
..\asm -o list.o list.asm
..\ln -v -o ..\ln\a.out main.o rtl.o string.o io.o os.o list.o

