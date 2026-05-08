default: prepare run

bin/Boot.bin: src/Boot.asm $(wildcard src/*.asm)
	nasm $< -f bin -o $@ -i src

bin/Extended.o: src/Extended.asm $(wildcard src/*.asm)
	nasm $< -f elf64 -o $@ -i src

bin/Binaries.o: src/Binaries.asm $(wildcard ./*.txt)
	nasm $< -f elf64 -o $@ -i src

bin/Kernel.o: src/Kernel.cpp $(wildcard src/*.hpp)
	clang --target=x86_64-elf -ffreestanding -mno-red-zone -m64 -c $< -o $@

bin/Kernel.bin: bin/Extended.o bin/Binaries.o bin/Kernel.o
	ld.lld -T Link.ld $^ -o $@

bin/Image.bin: bin/Boot.bin bin/Kernel.bin
	copy /b bin\Boot.bin+bin\Kernel.bin bin\Image.bin

run: bin/Image.bin
	qemu-system-x86_64 -drive if=floppy,format=raw,file=$<

prepare:
	if not exist bin mkdir bin

clean:
	if exist bin rmdir /s /q bin