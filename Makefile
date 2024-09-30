LD := $(CC)
all: fireworks
bin/fireworks.o: src/fireworks.c
	$(CC) -c $(CC_FLAGS) src/fireworks.c -o bin/fireworks.o
bin/main.o: src/main.c
	$(CC) -c $(CC_FLAGS) src/main.c -o bin/main.o
bin/rand.o: src/rand.c
	$(CC) -c $(CC_FLAGS) src/rand.c -o bin/rand.o
fireworks: bin/main.o bin/rand.o bin/fireworks.o
	$(LD) -ofireworks $(LD_FLAGS) bin/main.o bin/rand.o bin/fireworks.o -lm
clean:
	rm bin/*
run: fireworks
	@bash -c "function cleanup() { tput cnorm; clear; }; trap cleanup EXIT; tput civis; clear; ./fireworks"
