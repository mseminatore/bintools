# LN

The bintools linker.

## Using the linker

Using the linker is simple and straight forward. To link together two object
files into a single executable is as simple as the following.

```
bintools> ln main.o string.o
Linking...
main.o
string.o
Linking complete -> a.out
bintools>
```

By default and by convention the linker produces an executable named a.out. The
`-o` option can be used to provide an alternate outpuf file name. For example:

```
bintools> ln -o hello main.o string.o
Linking...
main.o
string.o
Linking complete -> hello
bintools>
```

By default the linker produces an executable image based at address 0. If you 
wish to change that, perhaps because you want to reserve the first 256 bytes 
of memory for a simple OS, you can use the `-b` option to provide a new base 
address.

```
bintools> ln -o hello -b 256 main.o string.o
Linking...
main.o
string.o
Linking complete -> hello
bintools>
```

## The linking process

The files specified on the command line are linked together in the order 
provided. This is important as the first code to be executed is the start
of the first object file provided.

The linker loads each of the specific object files, validating that they are
proper a.out files. then the 

## Linker generated variables

At link time the linker automatically generates a few extra symbols that can be
accessed from user code. These are as follows:

Symbol | Description
------ | -----------
__brk | Address of top of heap and bottom of stack in memory
__ram_start | Address is the start of heap memory
