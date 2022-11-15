DIRS = ln cisc dumpbin strip asm

all:
	set -e; for i in $(DIRS); do make -C $$i; done
	
clean:
	set -e; for i in $(DIRS); do make -C $$i clean; done
