#!/bin/bash


qemu-system-i386 -m 64M \
    -drive file=floppy.img,if=floppy,format=raw\
    -monitor stdio\
    -boot a
