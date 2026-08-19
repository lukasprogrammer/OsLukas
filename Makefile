OBJECTS = ./build/kernel.asm.o ./build/kernel.o ./build/terminal.o
FILES = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all:
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/terminal.c -o ./build/terminal.o
	i686-elf-ld -g -relocatable $(OBJECTS) -o ./build/completeKernel.o
	i686-elf-gcc -T ./linkerScript.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/completeKernel.o
	

	KERNEL_SIZE=$$(stat -c%s ./bin/kernel.bin); \
	KERNEL_SECTORS=$$((($$KERNEL_SIZE + 511) / 512)); \
	echo "Kernel size: $$KERNEL_SIZE bytes"; \
	echo "Kernel sectors: $$KERNEL_SECTORS"; \
	nasm -D KERNEL_SECTORS=$$KERNEL_SECTORS -f bin ./src/boot.asm -o ./bin/boot.bin

	cat ./bin/boot.bin ./bin/kernel.bin > ./bin/os.bin
	dd if=/dev/zero bs=512 count=8 >> ./bin/os.bin
clean:
	rm -f ./bin/boot.bin
	rm -f ./bin/kernel.bin
	rm -f ./bin/os.bin
	rm -f ./build/kernel.asm.o
	rm -f ./build/kernel.o
	rm -f ./build/completeKernel.o