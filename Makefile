AS86	=as86 -0 -a 
LD86	=ld86 -d -0 
AS 		=as 
LD 		=ld
LDFLAGS	=-m elf_i386 -Ttext=0x0 --oformat=binary
CC		=gcc
CFLAGS	=-m32 -ffreestanding -fno-builtin -fno-pic -fno-pie 
		
ARCHIVES=kernel/kernel.o mm/mm.o



.c.s:
	$(CC) $(CFLAGS) \
		-nostdinc -Iinclude -S -o $*.s $<

.s.o:
	$(AS) --32 -c -o $*.o $<

.c.o:
	$(CC) $(CFLAGS) \
		-nostdinc -Iinclude -c -o $*.o $<

all: image

tools/system: boot/head.o init/main.o \
	$(ARCHIVES) 
	$(LD) $(LDFLAGS) boot/head.o init/main.o \
	$(ARCHIVES) \
	-o tools/system > System.map

kernel/kernel.o:
	(cd kernel; make)

mm/mm.o:
	(cd mm; make)

boot/bootsect: boot/bootsect.s
	$(AS86) -o boot/bootsect.o boot/bootsect.s
	$(LD86) -s -o boot/bootsect boot/bootsect.o

boot/setup: boot/setup.s
	$(AS86) -o boot/setup.o boot/setup.s
	$(LD86) -s -o boot/setup boot/setup.o

image: boot/bootsect boot/setup tools/system 
	./image_script.sh



#tmp.s:	boot/bootsect.s tools/system
#	(echo -n "SYSSIZE = (";ls -l tools/system | grep system \
#		| cut -c25-31 | tr '\012' ' '; echo "+ 15 ) / 16") > tmp.s
#	cat boot/bootsect.s >> tmp.s

clean:
	rm -f floppy.img System.map tools/system boot/bootsect boot/setup
	rm -f init.*.o boot/*.o
	(cd mm; make clean)
	(cd kernel; make clean)


boot/head.o: boot/head.s

### Dependencies
init/main.o: init/main.c include/asm/io.h include/etc/simple_print.h \
	include/asm/system.h include/linux/sched.h