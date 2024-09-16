empty :=
space := $(empty) $(empty)
assembler := nasm
# cc := wsl clang --target=x86_64-elf
cc := wsl /usr/local/x86_64elfgcc/bin/x86_64-elf-gcc
# ld := wsl ld.lld
ld := wsl /usr/local/x86_64elfgcc/bin/x86_64-elf-ld
# objcopy := wsl llvm-objcopy
objcopy := wsl /usr/local/x86_64elfgcc/bin/x86_64-elf-objcopy
# emulator := bochs
emulator := qemu-system-x86_64

default: bin run

bin/Boot.bin: src/Boot.asm $(wildcard src/*.asm)
	$(assembler) $< -f bin -o $@ -i src

bin/Extended.o: src/Extended.asm $(wildcard src/*.asm)
	$(assembler) $< -f elf64 -o $@ -i src

bin/Binaries.o: src/Binaries.asm $(wildcard ./*.txt)
	$(assembler) $< -f elf64 -o $@ -i src

bin/Kernel.o: src/Kernel.cpp $(wildcard src/*.hpp)
	$(cc) -Ttext 0x8000 -ffreestanding -mno-red-zone -m64 -c $< -o $@

bin/Kernel.bin: bin/Extended.o bin/Binaries.o bin/Kernel.o
	$(ld) -T"Link.ld" $^ -o $@

# bin/Kernel.bin: bin/Kernel.tmp
#	$(objcopy) -O binary $< $@

bin/Image.bin: bin/Boot.bin bin/Kernel.bin
	copy /b $(subst $(space),+,$(subst /,\,$^)) $(subst /,\,$@)

run: bin/Image.bin
	$(emulator) -drive if=floppy,format=raw,file=$<
#	$(emulator) -q -f bochsrc.bxrc

bin:
	@mkdir "bin"

clean:
	@rmdir /s /q "bin"