# NanoComputer

A tiny 8-bit-register, 64 KB-memory virtual computer written in C++17: a
two-pass assembler, a step-by-step CPU, and a flat memory model.

* 8 general purpose 8-bit registers
* 64 KB byte-addressable memory
* 24 opcodes (MOV / ADD / SUB families, CMP, JZ / JNZ / JC / JNC / JMP, HALT)
* Labels, comments, hex or decimal literals in source
* Per-step register/flag trace plus machine-code and memory dumps

## Build

```sh
cmake -S . -B build
cmake --build build
```

The executable is `build/nanocomputer` (C++17, no external dependencies).

## Run

The program asks for the source path on **stdin**; it does not read `argv`:

```sh
echo test.asm | ./build/nanocomputer
```

Output: one line per executed instruction with PC, IR, ZF/CF and all
registers, followed by the first 64 bytes of assembled machine code and
dumps of `0x00F0-0x00FF` / `0x0100-0x010F`.

## Assembly syntax

```asm
start:                        ; label definition (own line or before an instruction)
    MOV AL, 10                ; mnemonic, operands
    MOV [0x0100], 0x2A        ; decimal or 0x-hex immediate
    ADD AL, 5                 ; trailing comment after ';'
    JNZ start
    HALT
```

* Mnemonics and register names must be **uppercase**
* `;` starts a comment; blank lines are ignored
* Addressing forms: `reg`, `imm`, `[0x1234]` (direct), `[reg]` (indirect)
* Labels are resolved in a two-pass scan, so forward jumps are fine

## Registers

| Name | AL | BL | CL | DL | AH | BH | CH | DH |
|---|---|---|---|---|---|---|---|---|
| Index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |

All registers are 8 bits wide. There are no 16-bit registers, so an address
used through `[reg]` is limited to `0x00`-`0xFF`.

## Instruction set

| Mnemonic | Opcode | Encoding | Operation | Sets flags |
|---|---|---|---|---|
| `MOV reg, imm` | `0x01` | `01 rr ii` | `reg = imm` | no |
| `ADD reg, imm` | `0x02` | `02 rr ii` | `reg += imm` | ZF |
| `SUB reg, imm` | `0x03` | `03 rr ii` | `reg -= imm` | ZF |
| `JNZ addr` | `0x04` | `04 ll hh` | jump if ZF = 0 | no |
| `JMP addr` | `0x05` | `05 ll hh` | unconditional jump | no |
| `CMP ra, rb` | `0x06` | `06 ra rb` | compare `ra` with `rb` | ZF, CF |
| `MOV reg, reg` | `0x07` | `07 dd ss` | `d = s` | no |
| `MOV [addr], imm` | `0x08` | `08 ll hh ii` | `mem[addr] = imm` | no |
| `MOV [addr], reg` | `0x09` | `09 ll hh rr` | `mem[addr] = reg` | no |
| `MOV reg, [addr]` | `0x0A` | `0A rr ll hh` | `reg = mem[addr]` | no |
| `ADD reg, reg` | `0x0B` | `0B dd ss` | `d += s` | ZF |
| `ADD [addr], imm` | `0x0C` | `0C ll hh ii` | `mem[addr] += imm` | ZF |
| `ADD [addr], reg` | `0x0D` | `0D ll hh rr` | `mem[addr] += reg` | ZF |
| `ADD reg, [addr]` | `0x0E` | `0E rr ll hh` | `reg += mem[addr]` | ZF |
| `SUB reg, reg` | `0x0F` | `0F dd ss` | `d -= s` | ZF |
| `SUB [addr], imm` | `0x10` | `10 ll hh ii` | `mem[addr] -= imm` | ZF |
| `SUB [addr], reg` | `0x11` | `11 ll hh rr` | `mem[addr] -= reg` | ZF |
| `SUB reg, [addr]` | `0x12` | `12 rr ll hh` | `reg -= mem[addr]` | ZF |
| `JZ addr` | `0x13` | `13 ll hh` | jump if ZF = 1 | no |
| `JC addr` | `0x14` | `14 ll hh` | jump if CF = 1 | no |
| `JNC addr` | `0x15` | `15 ll hh` | jump if CF = 0 | no |
| `MOV reg, [reg]` | `0x16` | `16 dd pp` | `d = mem[p]`, `p` = 8-bit index 0-7 | no |
| `MOV [reg], reg` | `0x17` | `17 pp ss` | `mem[p] = s` | no |
| `HALT` | `0xFF` | `FF` | stop the CPU | no |

`rr` / `dd` / `ss` = register index (0-7), `ii` = 8-bit immediate,
`ll hh` = 16-bit address, low byte first.

`CMP ra, rb` sets `CF = (ra < rb)` as **unsigned** comparison, and
`ZF = (ra == rb)`. There is no `CMP reg, imm`; load a register first.

## Memory layout

```
0x0000 - 0x????   program (always loaded at 0x0000, no ORG directive)
0x???? - 0x00FF   free for data *only* if you use [reg] addressing
0x0100 - 0xFFFF   free for data addressed with absolute [0x1234]
```

Two rules to remember:

1. **The program is always loaded at `0x0000`.** Writing data at a low
   address (e.g. `MOV [0x0010], 5`) will overwrite your own machine code if
   the program is long enough. Keep data above the end of the code.
2. **Indirect addressing reaches only `0x00`-`0xFF`**, because the pointer
   is an 8-bit register. So an array accessed through `[reg]` must live in
   the narrow window *above* the code and *below* `0x0100`.

## Example

```asm
; mem[0x0100] = 42, then add 8 and store the result at 0x0101
    MOV [0x0100], 42
    MOV AL, [0x0100]
    ADD AL, 8
    MOV [0x0101], AL
    HALT
```

```
Result 0x0100-0x010F: 2a 32 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                      ^^ ^^
                     42 50
```

## test.asm

`test.asm` exercises every implemented opcode and writes one result byte per
check into `0x0100`-`0x010F`. Run it and compare the `Result` line with the
table in the file header — all 16 bytes should match:

```
Result 0x0100-0x010F: aa 12 12 5a a5 0f 2c 07 0e 06 07 04 06 00 0f 77
```

`0x010D` is a failure bitmask for the three conditional jumps: `0` means
JZ, JC and JNC all branched correctly.

## Current limitations

* `ADD reg, [reg]` and `SUB reg, [reg]` are rejected by the assembler
  ("not implemented yet") — only the absolute-address forms exist.
* No `ORG`, no data segment, no `INC`/`DEC`/shifts/logic ops, no stack.
* Assembler errors are printed but do not abort: the offending line is
  skipped, later lines still assemble, and the process still exits with `0`.
  Messages appear twice because the source is scanned in two passes.
* Immediates are silently truncated to 8 bits (`MOV AL, 300` assembles to
  `44`); no range diagnostics.
* `CPU::step()` does not range-check register indices or PC, so a corrupted
  instruction stream can index outside the register file.
* `Memory::checkAddrAvailable()` never returns false, and 16-bit access at
  `0xFFFF` wraps to address `0`.
* The entry point is always `0x0000`; the path must come from stdin.

## License

Released under the [MIT License](LICENSE).

```
MIT License

Copyright (c) 2026 yileneko
```

