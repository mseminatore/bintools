# bintools

## What is bintools?

This project provides a collection of code and tools for exploring Object 
files. Specifically the [a.out](https://en.wikipedia.org/wiki/A.out) object 
file format used by early PDP-11 and UNIX operating systems. The a.out format
was used in Linux up until the 1.2 kernel.

> According to Wikipedia, the a.out file format was named as an abbreviated 
> form of "assembler output" and was the filename of the output from Ken 
> Thompson's PDP-7 assembler.

## Why bintools?

I've always been intrigued by the process of compiling, linking and executing
programs. While I've previously worked on compilers and interpreters, I had not
yet had the chance to work with object files and linkers.

I decided that the best way to learn more about linkers was to create one. In 
order to test the linker I needed a way to generate linkable object files. To 
keep the linker scope manageable I decided to use the `a.out` format. While 
the `a.out` format may be somewhat outdated, it is  well understood and 
documented. Once I understood the basic linking process I could always choose 
to re-implement the linker using a more modern object file format like `ELF` or
`PE`.

> If you want to learn about linkers and object file format, I highly recommend
> the book **Linkers & Loaders** by John R. Levine

Certainly, I could have chosen an existing assembler that produced `a.out` 
object files. But I also wanted to avoid large and complex instruction sets,
again to keep the linker scope manageable. Which lead to creating an  
assembler for a simple 8-bit CPU architecture that I defined.

And finally, I needed a way to prove that my linker had successfully produced 
a valid executable. For this I needed a CPU emulator that could load, run and
debug them.

## The bintools tools


* [asm](https://github.com/mseminatore/bintools/blob/master/asm/README.md) - an assembler that generates a.out object files
* [ln](https://github.com/mseminatore/bintools/blob/master/ln/README.md) - a linker which combines a.out object files into an executable
* [dumpbin](https://github.com/mseminatore/bintools/blob/master/dumpbin/README.md) - a tool to explore a.out object files and executables
* [cisc](https://github.com/mseminatore/bintools/blob/master/cisc/README.md) - an b-bit cpu simulator and debug monitor

