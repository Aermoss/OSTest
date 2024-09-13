cc := wsl /usr/local/x86_64elfgcc/bin/x86_64-elf-gcc

default:
	- mkdir "bin"
	nasm boot.asm -f bin -o bin/boot.bin
	nasm extended.asm -f elf64 -o bin/extended.o
	$(cc) -ffreestanding -mno-red-zone -m64 -c kernel.cpp -o bin/kernel.o
	ld -Ttext 0x8000 bin/extended.o bin/kernel.o -o bin/kernel.tmp
	objcopy -O binary bin/kernel.tmp bin/kernel.bin
	copy /b bin\boot.bin+bin\kernel.bin bin\final.bin
	qemu-system-x86_64 bin/final.bin

clean:
	- rmdir /s /q "bin"