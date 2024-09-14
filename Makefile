cc := wsl /usr/local/x86_64elfgcc/bin/x86_64-elf-gcc

default:
	- mkdir "bin"
	nasm Boot.asm -f bin -o bin/Boot.bin
	nasm Extended.asm -f elf64 -o bin/Extended.o
	$(cc) -ffreestanding -mno-red-zone -m64 -c Kernel.cpp -o bin/Kernel.o
	ld -Ttext 0x8000 bin/Extended.o bin/Kernel.o -o bin/Kernel.tmp
	objcopy -O binary bin/Kernel.tmp bin/Kernel.bin
	copy /b bin\Boot.bin+bin\Kernel.bin bin\Final.bin
	qemu-system-x86_64 bin/Final.bin

clean:
	- rmdir /s /q "bin"