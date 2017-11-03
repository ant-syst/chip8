FLAGS=-Wall -g -Wextra

all: bin/chip8_test.elf bin/sdl_io.elf bin/main.elf bin/disassemble.elf bin/debugger_test.elf

bin/*.elf: src/*.h

bin/main.elf: src/chip8.c src/main.c src/sdl_io.c src/args.c src/debugger.c src/tools.c src/disassemble.c src/jso.c src/debugger.c src/cJSON/cJSON.c
	gcc $(FLAGS) -lSDL2 -lrt $^ -o bin/main.elf

bin/chip8_test.elf: src/chip8.c src/chip8_test.c src/tools.c src/*.h
	gcc $(FLAGS) -lSDL2 -lrt src/chip8_test.c src/tools.c -o bin/chip8_test.elf

bin/sdl_io.elf: src/chip8.c src/sdl_io.c src/tools.c src/*.h
	gcc $(FLAGS) -lSDL2 -DTEST src/chip8.c src/sdl_io.c src/tools.c -o bin/sdl_io.elf

bin/disassemble.elf: src/disassemble.c
	gcc $(FLAGS) src/disassemble.c src/disassembler.c -o bin/disassemble.elf

bin/debugger_test.elf: src/debugger.c src/chip8.c src/debugger_test.c src/jso.c src/tools.c src/cJSON/cJSON.c
	gcc $(FLAGS) $^ -o $@

clean:
	rm bin/*

run_test: bin/chip8_test.elf
	./bin/chip8_test.elf

chip8_test_valgrind: bin/chip8_test.elf
	valgrind --tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=yes bin/chip8_test.elf

main_valgrind: bin/main.elf
	valgrind --tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=yes bin/main.elf
