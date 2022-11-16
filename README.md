# bintools

## What is bintools?

This project provides a collection of code and tools for exploring and working
with Object files. Specifically the 
[a.out](https://en.wikipedia.org/wiki/A.out) object file format used by early 
PDP-11 and UNIX operating systems. The `a.out` format was used in Linux up 
until the 1.2 kernel.

> According to Wikipedia, the `a.out` file format name was an abbreviation of 
> "assembler output" and was the default output filename from Ken Thompson's 
> PDP-7 assembler.

The bintools project is built regularly on Windows using Visual Studio as well
as Mac OSX using Clang.

## Why bintools?

I am fascinated by the process of compiling, linking and executing programs. 
Linkers had always seemed complex to the point of being a bit magical. While 
I had previously worked on compilers and interpreters, I had not yet had the 
opportunity to work directly with object files and linkers.

It seemed clear that the best way to learn a lot more about linkers and the 
linking process was to create one. To keep the linker scope 
manageable the `a.out` format was chosen. While the `a.out` format may be 
considered somewhat outdated, it benefits from being well understood and 
documented. Once the basic linking process was understood it would always be 
an option to re-implement the linker using a more modern object file format 
like `ELF` or `PE`.

> If you want to learn more about linkers and object file formats, I highly 
> recommend the book '**Linkers & Loaders**' by John R. Levine

In order to test the linker it was necessary to have a way to generate linkable
object files. Certainly, it would have been possible to chose an existing 
assembler that produced `a.out` object files. To keep the linker scope more 
manageable it seemed important to avoid large and complex instruction sets. 
Which lead to the need to create a simple 8-bit CPU architecture and an 
assembler for it.

And finally, a way was to prove that the linker had successfully produced a 
valid executable. For this a CPU emulator that could load, run and
debug executables was created.

## The bintools tools

The bintools project consists of a number of tools. As mentioned above most of 
the tools were created in order to provide the input, and test the output of 
the linker.

The current set of tools are:

* [asm](https://github.com/mseminatore/bintools/blob/master/asm) - an assembler that generates a.out object files
* [ln](https://github.com/mseminatore/bintools/blob/master/ln) - a linker which combines a.out object files into an executable
* [dumpbin](https://github.com/mseminatore/bintools/blob/master/dumpbin) - a utility to explore a.out object files and executables
* [cisc](https://github.com/mseminatore/bintools/blob/master/cisc/) - an 8-bit CPU simulator and debug monitor
* [strip](https://github.com/mseminatore/bintools/blob/master/strip/) - utility to strip symbols and relocation data
