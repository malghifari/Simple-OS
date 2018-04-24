#!/bin/sh
set -e

dd if=/dev/zero of=floppya.img bs=512 count=2880
nasm bootload.asm
dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
dd if=map.img of=floppya.img bs=512 count=1 seek=256 conv=notrunc
dd if=files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc
dd if=sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc

echo "Compiling kernel"
bcc -ansi -c -o proc.o proc.c
bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o proc.o

dd if=kernel of=floppya.img bs=512 conv=notrunc seek=1 iflag=fullblock

as86 lib.asm -o lib_asm.o
bcc -ansi -c -o helper.o helper.c

bcc -ansi -c shell.o shell.c
ld86 -o shell -d shell.o lib_asm.o helper.o
./loadFile shell

bcc -ansi -c ls.o ls.c
ld86 -o ls -d ls.o lib_asm.o helper.o
./loadFile ls

bcc -ansi -c mkdir.o mkdir.c
ld86 -o mkdir -d mkdir.o lib_asm.o helper.o
./loadFile mkdir

bcc -ansi -c touch.o touch.c
ld86 -o touch -d touch.o lib_asm.o helper.o
./loadFile touch

bcc -ansi -c cat.o cat.c
ld86 -o cat -d cat.o lib_asm.o helper.o
./loadFile cat

./loadFile keyproc2

bochs -f opsys.bxrc
