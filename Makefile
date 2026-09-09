OBJECTS = \
	./build/kernel.asm.o \
	./build/interrupts.asm.o \
	./build/switch.asm.o \
	./build/gdt.asm.o \
	./build/kernel.o \
	./build/terminal.o \
	./build/idt.o \
	./build/interrupts.o \
	./build/pic.o \
	./build/pit.o \
	./build/keyboard.o \
	./build/shell.o \
	./build/memory.o \
	./build/paging.o \
	./build/heap.o \
	./build/task.o \
	./build/gdt.o \
	./build/tss.o\
	./build/graphics.o

all:

	nasm -f elf32 ./src/user.asm -o ./build/user.o

	i686-elf-ld \
		-T ./user_linker.ld \
		-o ./build/user.elf \
		./build/user.o

	i686-elf-objcopy \
		-O binary \
		./build/user.elf \
		./bin/user.bin

	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o
	nasm -f elf -g ./src/interrupts.asm -o ./build/interrupts.asm.o
	nasm -f elf -g ./src/switch.asm -o ./build/switch.asm.o
	nasm -f elf -g ./src/gdt.asm -o ./build/gdt.asm.o

	i686-elf-gcc -I./src -std=gnu99 -c ./src/interrupts.c -o ./build/interrupts.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/terminal.c -o ./build/terminal.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/idt.c -o ./build/idt.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/pic.c -o ./build/pic.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/pit.c -o ./build/pit.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/keyboard.c -o ./build/keyboard.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/shell.c -o ./build/shell.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/memory.c -o ./build/memory.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/paging.c -o ./build/paging.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/heap.c -o ./build/heap.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/task.c -o ./build/task.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/gdt.c -o ./build/gdt.o
	i686-elf-gcc -I./src -std=gnu99 -c ./src/tss.c -o ./build/tss.o
	i686-elf-gcc -I./src -I./src/graphics -std=gnu99 -c ./src/graphics/graphics.c -o ./build/graphics.o



	i686-elf-ld -g -relocatable $(OBJECTS) -o ./build/completeKernel.o

	i686-elf-gcc \
		-T ./linkerScript.ld \
		-o ./bin/kernel.bin \
		-ffreestanding \
		-O0 \
		-nostdlib \
		./build/completeKernel.o

	KERNEL_SIZE=$$(stat -c%s ./bin/kernel.bin); \
	KERNEL_SECTORS=$$((($$KERNEL_SIZE + 511) / 512)); \
	KERNEL_PADDED_SIZE=$$(($$KERNEL_SECTORS * 512)); \
	KERNEL_PADDING=$$(($$KERNEL_PADDED_SIZE - $$KERNEL_SIZE)); \
	\
	USER_SIZE=$$(stat -c%s ./bin/user.bin); \
	USER_SECTORS=$$((($$USER_SIZE + 511) / 512)); \
	KERNEL_LBA=2; \
	USER_LBA=$$(($$KERNEL_LBA + $$KERNEL_SECTORS)); \
	\
	echo "Kernel size: $$KERNEL_SIZE bytes"; \
	echo "Kernel sectors: $$KERNEL_SECTORS"; \
	echo "Kernel LBA: $$KERNEL_LBA"; \
	echo "User size: $$USER_SIZE bytes"; \
	echo "User sectors: $$USER_SECTORS"; \
	echo "User LBA: $$USER_LBA"; \
	\
	nasm \
		-D KERNEL_SECTORS=$$KERNEL_SECTORS \
		-D USER_SECTORS=$$USER_SECTORS \
		-D USER_LBA=$$USER_LBA \
		-D USER_SIZE=$$USER_SIZE \
		-f bin \
		./src/boot.asm \
		-o ./bin/boot.bin; \
	\
	nasm \
		-D KERNEL_SECTORS=$$KERNEL_SECTORS \
		-D USER_SECTORS=$$USER_SECTORS \
		-D USER_LBA=$$USER_LBA \
		-D USER_SIZE=$$USER_SIZE \
		-f bin \
		./src/stage2.asm \
		-o ./bin/stage2.bin; \
	\
	cp ./bin/boot.bin ./bin/os.bin; \
	cat ./bin/stage2.bin >> ./bin/os.bin; \
	cat ./bin/kernel.bin >> ./bin/os.bin; \
	dd if=/dev/zero bs=1 count=$$KERNEL_PADDING >> ./bin/os.bin 2>/dev/null; \
	cat ./bin/user.bin >> ./bin/os.bin; \
	dd if=/dev/zero bs=512 count=8 >> ./bin/os.bin 2>/dev/null

run:
	./build.sh
	qemu-system-i386 -drive file=./bin/os.bin,format=raw

clean:
	rm -f ./bin/stage2.bin
	rm -f ./bin/boot.bin
	rm -f ./bin/kernel.bin
	rm -f ./bin/os.bin

	rm -f ./build/kernel.asm.o
	rm -f ./build/interrupts.asm.o
	rm -f ./build/switch.asm.o
	rm -f ./build/gdt.asm.o

	rm -f ./build/kernel.o
	rm -f ./build/terminal.o
	rm -f ./build/idt.o
	rm -f ./build/interrupts.o
	rm -f ./build/pic.o
	rm -f ./build/pit.o
	rm -f ./build/keyboard.o
	rm -f ./build/shell.o
	rm -f ./build/memory.o
	rm -f ./build/paging.o
	rm -f ./build/heap.o
	rm -f ./build/task.o
	rm -f ./build/gdt.o
	rm -f ./build/tss.o
	rm -f ./build/graphics.o

	rm -f ./build/completeKernel.o
	rm -f ./bin/user.bin
	rm -f ./build/user.o
	rm -f ./build/user.elf
	