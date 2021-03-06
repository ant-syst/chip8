CC=gcc
CFLAGS=-Wall -g -Wextra -MMD

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c, obj/%.o, ${SRCS})
DEPS := $(wildcard obj/*.d)

all:  bin/main.elf bin/chip8_test.elf bin/sdl_io_test.elf bin/console_io_test.elf \
	bin/disassemble.elf bin/disassemble.so bin/debugger_test.elf bin/jso_test.elf

bin:
	mkdir -p bin

obj:
	mkdir -p obj

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -Ilib -o $@ -c $<

bin/main.elf: obj/chip8.o obj/main.o obj/vm.o obj/sdl_io.o obj/args.o obj/debugger.o obj/disassemble.o obj/jso.o obj/debugger.o lib/cJSON/cJSON.c obj/console_io.o obj/mapping.o | bin
	$(CC) $^ -lSDL2 -lrt -lncursesw -Ilib/ -o $@

bin/chip8_test.elf: obj/chip8_test.o | bin
	$(CC) $^ -lSDL2 -lrt -o $@

bin/sdl_io_test.elf: obj/chip8.o obj/sdl_io.o obj/mapping.o obj/sdl_io_test.o | bin
	$(CC) $^ -lSDL2 -o $@

bin/console_io_test.elf: obj/chip8.o obj/mapping.o obj/console_io.o obj/console_io_test.o | bin
	$(CC) $^ -lncursesw -o $@

bin/disassemble.so: obj/disassemble.o | bin
	$(CC) -shared -Wl,-soname,disassemble -o $@ -fPIC $^

bin/disassemble.elf: obj/disassemble.o obj/disassembler.o | bin
	$(CC) $^ -o $@

bin/debugger_test.elf: obj/debugger.o obj/chip8.o obj/debugger_test.o obj/jso.o lib/cJSON/cJSON.c | bin
	$(CC) -Ilib/ $^ -o $@

bin/jso_test.elf: obj/jso.o obj/jso_test.o | bin
	$(CC)  $^ -o $@

include ${DEPS}

.PHONY: clean mrproper run_tests chip8_tests_valgrind bin obj

clean:
	rm -f obj/*

mrproper:
	rm -f bin/*

run_tests: bin/chip8_test.elf
	./bin/chip8_test.elf

chip8_tests_valgrind: bin/chip8_test.elf
	valgrind --tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=yes bin/chip8_test.elf

main_valgrind: bin/main.elf
	valgrind --tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=yes bin/main.elf
