# bintools

## What is bintools?

This project provides a collection of code and tools for exploring Object 
files. Specifically the [a.out](https://en.wikipedia.org/wiki/A.out) object 
file format used by many Unix operating systems.

## Why bintools?

I've always been intrigued by the process of compiling, linking and executing
programs. While I've previously worked on compilers and interpreters I had not
yet had the chance to work with object files and linkers.

> If you want to learn about linkers I highly recommend the book 
> 'Linkers & Loaders' by John R. Levine

## The tools

[asm](asm/readme.md) - an assembler that generates a.out object files
[ln](ln/readme.md) - a linker which combines a.out object files into an executable
[dumpbin](dumpbin/readme.md) - a tool to explore a.out object files and executables
