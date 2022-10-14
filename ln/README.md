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
-o option can be used to provide an alternate outpuf file name. For example:

```
bintools> ln -o hello main.o string.o
```
