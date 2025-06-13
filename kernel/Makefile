CC=gcc
CFLAGS=-m32 -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -I../include
AS	=as
LD	=ld
LDFLAGS=-m elf_i386 -Ttext=0x0

.c.s:
	$(CC) $(CFLAGS) \
	-S -o $*.s $<

.s.o:
	$(AS) --32 -c -o $*.o $<

.c.o:
	$(CC) $(CFLAGS) \
	-c -o $*.o $<

OBJS= traps.o asm.o

all: kernel.o

kernel.o: $(OBJS)
	$(LD) $(LDFLAGS) -r -o kernel.o $(OBJS)
	sync
clean:
	rm -f *.o *.a

#traps.s traps.o : traps.c ../include/linux/head.h ../include/etc/simple_print.h \
#	../include/asm/system.h