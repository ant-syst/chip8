# Chip8 #

This repository contains an implementation of a [chip8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) emulator/disassembler/debugger. The emulator is written in C using either SDL or ncurses for rendering. The disassembler/debugger programs are written in python with parts in C.

## Compilation ##

The emulator requires the following libraries to be installed : `ncurses`, `sdl2`.

Compilation setup :
```bash  
./build.sh
make all
```
## Emulator ##

Keyboard mapping : 

```
Keypad                   Keyboard
+-+-+-+-+                +-+-+-+-+
|1|2|3|C|                |1|2|3|4|
+-+-+-+-+                +-+-+-+-+
|4|5|6|D|                |A|Z|E|R|
+-+-+-+-+       =>       +-+-+-+-+
|7|8|9|E|                |Q|S|D|F|
+-+-+-+-+                +-+-+-+-+
|A|0|B|F|                |W|X|C|V|
+-+-+-+-+                +-+-+-+-+
```

A list of open sources ROMs are present in `roms` folder

Command to run the emulator using `sdl` library
```bash
 ./bin/main.elf --rom ./roms/MAZE --io sdl
```

Command to run the emulator in terminal using `ncurses` library
```bash
 ./bin/main.elf --rom ./roms/MAZE --io curses
```

## Disassembler ##

### Setup ###

The disassembler is made in python with few calls done to C library. The list of requirements used by the disassembler are present in `requirements.txt`.

### Challenges ###

Chip8 instructions are 2 bytes long and branch instructions can jump to any aligned or unaligned address (see first instruction of roms MISSILE, MERLIN, INVADERS, HIDDEN, BLITZ). Therefore any combination of two bytes that correspond to a valid instruction can be used by a ROM. In order to cope with such behavior, we choose different approaches :

1. The two column approach displays the source code in two columns the left for paired addresses instructions, the right for impaired addresses instructions. The reader decodes the program by reading and choosing which column is the good one.

2. The sprites approach print the source code as sprite in order to help the detection of sprites

3. The enhanced approach uses additional information (list of instruction address, list of sprites address, list of instruction address that are used as branching target) to perform disassembling.

### Two columns ###

Disassemble Chip8 rom in two columns, 

```bash  
./tools/disassemble.py --rom roms/PONG instructions
```

### Sprites ###

Disassemble rom as if it was only composed of sprites.

```bash
./tools/disassemble.py --rom roms/INVADERS sprites
```

### Enhanced approach ###

Disassemble the rom using additional information to identify the different parts of the program. The emulator option `--instructions-logger-path` could be used to generate a file containing the address of the executed instructions.

Command :
```bash
./tools/disassemble.py --rom ../chip8/roms/MAZE infos --its-addrs-path ./roms/MAZE_it --labels-path ./roms/MAZE_label --data-addrs-path ./roms/MAZE_data
```

Results :
```
0x200: 0xa21e I <- 0x21e        		
0x202: 0xc201 v2 <- rand() and 1		
0x204: 0x3201 skip if v2 == 1   		
0x206: 0xa21a I <- 0x21a        		
0x208: 0xd014 sprite at v0:v1 4 		
0x20a: 0x7004 v0 <- v0 + 4      		
0x20c: 0x3040 skip if v0 == 64  		
0x20e: 0x1200 jump 0x200        		
0x210: 0x6000 v0 <- 0           		
0x212: 0x7104 v1 <- v1 + 4      		
0x214: 0x3120 skip if v1 == 32  		
0x216: 0x1200 jump 0x200        		
0x218: 0x1218 jump 0x218        		
0x21a: 0x80 █                   		
0x21b: 0x40  █                  		
0x21c: 0x20   █                 		
0x21d: 0x10    █                		
0x21e: 0x20   █                 		
0x21f: 0x40  █                  		
0x220: 0x80 █  
```

## Debugging ##

The emulator contains a debugger mode. When the emulator runs in this mode, it creates a UNIX socket at `/var/run/chip8/dbg.sock` path stop the program and waits for debugger connection. The folder `/var/run/chip8/` must exists and write permission must be granted to the emulator program.

Creation of the folder that owns the socket path :
```bash
sudo mkdir /var/run/chip8/
sudo chown $USER:$USER /var/run/chip8/
```

Run the emulator in debug mode :
```bash
./bin/main.elf --rom ./roms/MAZE --io sdl --use-debug
```

Connect the debugger to the emulator :
```bash
./tools/dbg_client.py
(dbg): 
```

Type `help` to get a list of commands.

## Developpement ##

The unit tests done are present in the `bin` folder suffixed with `test.elf`.
