LD := $(CC)
all: fireworks
bin/fireworks.o: fireworks.c
	$(CC) -c $(CC_FLAGS) fireworks.c -o bin/fireworks.o
bin/main.o: main.c
	$(CC) -c $(CC_FLAGS) main.c -o bin/main.o
bin/rand.o: rand.c
	$(CC) -c $(CC_FLAGS) rand.c -o bin/rand.o
fireworks: bin/main.o bin/rand.o bin/fireworks.o
	$(LD) -ofireworks $(LD_FLAGS) bin/main.o bin/rand.o bin/fireworks.o -lm
clean:
	rm bin/*
run: fireworks
	@bash -c "function cleanup() { tput cnorm; }; trap cleanup EXIT; tput civis; clear; ./fireworks"
