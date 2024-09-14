empty :=
space := $(empty) $(empty)
cc := wsl /usr/local/x86_64elfgcc/bin/x86_64-elf-gcc
emulator := qemu-system-x86_64

default: bin run

bin/Boot.bin: Boot.asm $(wildcard *.asm)
	nasm $< -f bin -o $@

bin/Extended.o: Extended.asm $(wildcard *.asm)
	nasm $< -f elf64 -o $@

bin/Kernel.o: Kernel.cpp $(wildcard *.hpp)
	$(cc) -ffreestanding -mno-red-zone -m64 -c $< -o $@

bin/Kernel.tmp: bin/Extended.o bin/Kernel.o
	ld -Ttext 0x8000 $^ -o $@

bin/Kernel.bin: bin/Kernel.tmp
	objcopy -O binary $< $@

bin/Final.bin: bin/Boot.bin bin/Kernel.bin
	copy /b $(subst $(space),+,$(subst /,\,$^)) $(subst /,\,$@)

run: bin/Final.bin
	$(emulator) $<

bin:
	@mkdir "bin"

clean:
	@rmdir /s /q "bin"