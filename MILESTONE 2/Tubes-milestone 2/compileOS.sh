dd if=/dev/zero of=floppya.img bs=512 count=2880
nasm tools/bootload.asm
dd if=tools/bootload of=floppya.img bs=512 count=1 conv=notrunc
dd if=map.img of=floppya.img bs=512 count=1 seek=256 conv=notrunc
dd if=files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc 
dd if=sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc
bcc -ansi -c -o kernel.o kernel.c

as86 tools/kernel.asm -o tools/kernel_asm.o
as86 tools/lib.asm -o tools/lib.o

ld86 -o kernel -d kernel.o tools/kernel_asm.o
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=1

bcc -ansi -c -o tools/shell/shell.o tools/shell/shell.c
ld86 -o shell -d tools/shell/shell.o tools/lib.o
./loadFile shell

bcc -ansi -c -o tools/echo/echo.o tools/echo/echo.c
ld86 -o echo -d tools/echo/echo.o tools/lib.o
./loadFile echo

bcc -ansi -c -o tools/ls/ls.o tools/ls/ls.c
ld86 -o ls -d tools/ls/ls.o tools/lib.o
./loadFile ls

bcc -ansi -c -o tools/mkdir/mkdir.o tools/mkdir/mkdir.c
ld86 -o mkdir -d tools/mkdir/mkdir.o tools/lib.o
./loadFile mkdir

bcc -ansi -c -o tools/rm/rm.o tools/rm/rm.c
ld86 -o rm -d tools/rm/rm.o tools/lib.o
./loadFile rm

bcc -ansi -c -o tools/cat/cat.o tools/cat/cat.c
ld86 -o cat -d tools/cat/cat.o tools/lib.o
./loadFile cat

./loadFile keyproc2