DIRS = asm ln cisc dumpbin

all:
	set -e; for i in $(DIRS); do make -C $$i; done
	
clean:
	set -e; for i in $(DIRS); do make -C $$i clean; done
