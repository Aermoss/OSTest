default:
	nasm boot.asm -f bin -o bin/boot.bin
	nasm extended.asm -f elf64 -o bin/extended.o
	gcc -ffreestanding -mno-red-zone -m64 -c kernel.cpp -o bin/kernel.o
	ld -Ttext 0x8000 bin/extended.o bin/kernel.o -o bin/kernel.tmp
	objcopy -O binary bin/kernel.tmp bin/kernel.bin
	copy /b bin\boot.bin+bin\kernel.bin bin\final.bin
	qemu-system-x86_64 bin/final.bin