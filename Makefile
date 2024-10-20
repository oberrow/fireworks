LD := $(CC)
NASM := nasm
all: fireworks
bin/fireworks.o: src/fireworks.c
	$(CC) -c $(CC_FLAGS) src/fireworks.c -o bin/fireworks.o
bin/main.o: src/main.c
	$(CC) -c $(CC_FLAGS) src/main.c -o bin/main.o
bin/rand.o: src/rand.c
	$(CC) -c $(CC_FLAGS) src/rand.c -o bin/rand.o
bin/x86_64-randseed.o: src/x86_64-randseed.asm
	$(NASM) $(NASM_FLAGS) -felf64 src/x86_64-randseed.asm -o bin/x86_64-randseed.o
fireworks: bin/main.o bin/rand.o bin/fireworks.o bin/x86_64-randseed.o
	$(LD) -ofireworks $(LD_FLAGS) bin/main.o bin/rand.o bin/fireworks.o bin/x86_64-randseed.o -lm
clean:
	rm bin/*
run: fireworks
	@bash -c "function cleanup() { tput cnorm; clear; }; trap cleanup EXIT; tput civis; clear; ./fireworks"
