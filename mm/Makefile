CC=gcc
CFLAGS=-m32 -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -I../include
AS	=as
LD	=ld
LDFLAGS=-m elf_i386 -Ttext=0x0
.c.o:
	$(CC) $(CFLAGS) \
	-c -o $*.o $<

.s.o: $(AS) -o $*.o $<

.c.s:
	$(CC) $(CFLAGS) \
	-S -o $*.s $<

OBJS	=memory.o

all: mm.o
mm.o: $(OBJS)
	$(LD) $(LDFLAGS) -r -o mm.o $(OBJS)

clean:
	rm -f *.o *.a

memory.o: memory.c 