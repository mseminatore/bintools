# STRIP

The bintools object file strip utility.

## Using strip

To strip the symbol data from an object file:

```
bintools> strip a.o

Strip complete -> a.out

bintools>
```

To remove relocation data as well as symbols:

```
bintools> strip -r a.o

Strip complete -> a.out

bintools>
```

To remove all non-executable information:

```
bintools> strip -a a.o

Strip complete -> a.out

bintools>
```

And finally to set the outpub file name:

```
bintools> strip -o a_stripped.o -a a.o

Strip complete -> a_stripped.out

bintools>
```
