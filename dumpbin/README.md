# DUMPBIN

The bintools dumpbin tool. A tool for viewing the internals of an `a.out`
formatted object file.

## Using dumpbin

Like other bintools, using `dumpbin` is simple and straight forward. Running
the tool without any arguments displays the various command line options.

```
bintools> dumpbin
usage: dumpbin [options] filename

-t      dump text segment
-d      dump data segment
-r      dump relocations
-s      dump symbols
-a      dump all
```

You can view the see details of a particular object file as follows:

```
bintools> dumpbin main.o
Dump of file: main.o

File Type: OBJECT FILE

File Header
-----------

     Magic Number: 0x107
Text Segment size: 0x00BC (188) bytes
Data Segment size: 0x0012 (18) bytes
 BSS Segment size: 0x0000 (0) bytes
Symbol Table size: 0x0014 (20) bytes
 Main Entry Point: 0x0000
 Text reloc count: 0x0019 (25) entries
 Data reloc count: 0x0000 (0) entries
 ```

The first thing `dumpbin` does is to check that the file appears to be a valid
`a.out` file. If not a warning is displayed and `dumpbin` terminates.

Then `dumpbin` determines whether the file is an OBJECT file or
an EXECUTABLE file. 

> Since UNIX object file formats are also valid executables files the only
> real difference between OBJECT and EXECUTABLE files is the existence of 
> relocation table entries.

Finally, `dumpbin` reports the details of the various segments, symbol tables 
and relocation entries. 

> For historical reasons the code segment is called 
> `text` and the uninitialized data segments is called `bss`.

## Displaying segment contents

Using the `-t` and/or `-d` options will also dump the contents of the text and/or
data segments.

> Note that segments, especially in a fully linked executable image, can be 
> quite large!

```
bintools> dumpbin -t main.o
Dump of file: main.o

File Type: OBJECT FILE

File Header
-----------

     Magic Number: 0x107
Text Segment size: 0x00BC (188) bytes
Data Segment size: 0x0012 (18) bytes
 BSS Segment size: 0x0000 (0) bytes
Symbol Table size: 0x0014 (20) bytes
 Main Entry Point: 0x0000
 Text reloc count: 0x0019 (25) entries
 Data reloc count: 0x0000 (0) entries

Text segment (hex)
------------------

0000: 0D 00 00 14  0A 0D 00 00 - 16 00 00 18  16 0C 00 1D [................]
0010: 02 16 06 00  1D 02 0D 00 - 00 16 06 00  0D 00 00 16 [................]
0020: 06 00 0D 00  00 0D A0 00 - 0D 3A 00 0D  5A 00 0D 61 [.........:..Z..a]
0030: 00 0D 6B 00  0D 93 00 10 - 37 00 02 17  02 63 02 FF [..k.....7....c..]
0040: 02 10 01 00  01 01 00 00 - 04 17 04 61  04 01 04 10 [...........a....]
0050: 03 FF FF 03  10 00 03 03 - 00 0E 0D 00  00 0D AC 00 [................]
0060: 0E 06 FF 05  10 00 08 01 - 0C FF 0E 13  FF FF 13 10 [................]
0070: 00 14 10 16  00 01 18 19 - FE FF 19 03  00 16 FE FF [................]
0080: 15 FE FF 16  FE FF 15 FE - FF 16 04 00  15 04 00 1A [................]
0090: FE FF 0E 1D  01 1E 01 1D - 03 1D 09 1E  09 1E 03 0E [................]
00A0: 14 03 1D 01  1E 01 04 01 - 11 A2 00 0E  1D 02 1D 04 [................]
00B0: 1E 02 17 0C  17 F4 17 10 - 18 1E 02 0E  00 00 00 00 [................]

```

## Displaying relocation entries

Using the `-r` option will dump the contents of the text and data relocation
tables. Relocation table entries mark the location of references to code and
data that the linker will have to fixup when code/data segments are moved and
when the address of external symbols are resolved.

```
bintools> dumpbin -r main.o
Dump of file: main.o

File Type: OBJECT FILE

File Header
-----------

     Magic Number: 0x107
Text Segment size: 0x00BC (188) bytes
Data Segment size: 0x0012 (18) bytes
 BSS Segment size: 0x0000 (0) bytes
Symbol Table size: 0x0014 (20) bytes
 Main Entry Point: 0x0000
 Text reloc count: 0x0019 (25) entries
 Data reloc count: 0x0000 (0) entries

.text segment relocations
-------------------------

0001    size: 2 (bytes) External        _rtlInit
0006    size: 2 (bytes) External        malloc
0009    size: 2 (bytes) External        systick
000D    size: 2 (bytes) Segment: .data
0012    size: 2 (bytes) Segment: .data
0017    size: 2 (bytes) External        _strcpy
001A    size: 2 (bytes) Segment: .data
001D    size: 2 (bytes) External        _strlen
0020    size: 2 (bytes) Segment: .data
0023    size: 2 (bytes) External        _zeromem
0026    size: 2 (bytes) Segment: .text
0029    size: 2 (bytes) Segment: .text
002C    size: 2 (bytes) Segment: .text
002F    size: 2 (bytes) Segment: .text
0032    size: 2 (bytes) Segment: .text
0035    size: 2 (bytes) Segment: .text
0038    size: 2 (bytes) Segment: .text
0046    size: 2 (bytes) Segment: .data
0057    size: 2 (bytes) Segment: .data
005B    size: 2 (bytes) External        _strlen
005E    size: 2 (bytes) Segment: .text
007B    size: 2 (bytes) Segment: .data
008A    size: 2 (bytes) Segment: .data
008D    size: 2 (bytes) Segment: .data
00A9    size: 2 (bytes) Segment: .text
```

## Displaying symbol table entries

And finally the `-s` option will dump the contents of the symbol table. The 
symbol table lists all the named memory locations in the code/data segments.
When the linker needs to resolve an external symbol by name it uses the
symbol table to find its address for patching relocation entries.

```
bintools> dumpbin -s main.o
Dump of file: main.o

File Type: OBJECT FILE

File Header
-----------

     Magic Number: 0x107
Text Segment size: 0x00BC (188) bytes
Data Segment size: 0x0012 (18) bytes
 BSS Segment size: 0x0000 (0) bytes
Symbol Table size: 0x0014 (20) bytes
 Main Entry Point: 0x0000
 Text reloc count: 0x0019 (25) entries
 Data reloc count: 0x0000 (0) entries

Symbols
-------

       _zeromem  public external segment offset: 0 (0x0000)
       _rtlInit  public external segment offset: 0 (0x0000)
         malloc  public external segment offset: 0 (0x0000)
        systick  public external segment offset: 0 (0x0000)
        _strlen  public external segment offset: 0 (0x0000)
        _strcpy  public external segment offset: 0 (0x0000)
          lives  .data segment offset: 0 (0x0000)
         switch  .data segment offset: 2 (0x0002)
          timer  .data segment offset: 3 (0x0003)
          score  .data segment offset: 4 (0x0004)
         prompt  .data segment offset: 6 (0x0006)
         target  .data segment offset: 12 (0x000C)
          _main  .text segment offset: 0 (0x0000)
       testMath  .text segment offset: 58 (0x003A)
   testBranches  .text segment offset: 90 (0x005A)
      testLogic  .text segment offset: 97 (0x0061)
  testLoadStore  .text segment offset: 107 (0x006B)
      testStack  .text segment offset: 147 (0x0093)
    testForLoop  .text segment offset: 160 (0x00A0)
           _foo  .text segment offset: 172 (0x00AC)
```