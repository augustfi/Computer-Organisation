# Computer-Organisation

Runnable master files for IS1500 Computer Organization and Components, one per module.
Each file collects that module's exam questions as code you can compile and run.

| Module | Topic | File | Status |
|---|---|---|---|
| 1 | C and Assembly Programming, C part | `module1_c_programming.c` | in progress |
| 1 | C and Assembly Programming, machine-code part | `module1_machine_code.c` | 13 items |
| 1 | C and Assembly Programming, assembly | `module1_assembly.asm` | 15 items, not yet run |
| 2 | I/O Systems | | not started |
| 3 | Logic Design | | not started |
| 4 | Processor Design | | not started |
| 5 | Memory Hierarchy | | not started |
| 6 | Parallel Processors and Programs | | not started |

## Module 1

Each exam item is its own function under an `Item N` header. `main()` calls one item at a
time; uncomment the one you want to run.

```
gcc -Wno-pointer-to-int-cast module1_c_programming.c -o module1 && ./module1
```

Lines marked `ADDED` are the answers filled in for the exam's missing code. Lines marked
`ADDED (not in the exam)` are extra prints used to check an answer.

The exam assumes 32-bit RISC-V. On a 64-bit computer the `(int)p` casts give a harmless
"cast to smaller integer type" warning, which the flag above hides.

## Module 1, machine code

The second part of Q1: encoding and decoding RISC-V instructions. Each item builds the
machine code field by field, prints the bits, and compares the result with the exam's answer.

```
gcc module1_machine_code.c -o machine && ./machine
```

Items 1 to 7 come from MIPS exams and are done in their RISC-V version.

## Module 1, assembly

`module1_assembly.asm` has the same exam items as real RISC-V assembly. Every item is its
own function, and `main` calls them in order. Put a `#` in front of a call to skip that item.
The instruction each exam question asks about is marked `<-- EXAM` with its machine code.

It runs in RARS, a RISC-V simulator: open the file, assemble (F3), then run (F5). The
"Code" column shows the machine code of every instruction.

```
java -jar rars1_6.jar module1_assembly.asm
```

| Call in `main` | Task |
|---|---|
| `jal item1a` | Call `foo` (the assembly function translated to C in the C file) |
| `jal item1b` | `sw t3,-8(a0)` (S-type) |
| `jal item2a` | `addi t3,s0,-7`, decoded from `0xFF940E13` |
| `jal item3b` | `lw s1,8(t0)`, offset from two addresses |
| `jal item4a` | `xor s0,t2,t3`: which register changes |
| `jal item5b` | `lw s6,-11(t6)` and its bytes in little-endian memory |
| `jal item6b` | `bne t1,zero,loop` (B-type, offset -4) |
| `jal item7a` | `addi t3,t6,-23` |
| `jal item8a` | `sll t2,t0,s1`: t2 becomes 48 |
| `jal item9a` | `sra t0,t1,t2`: t0 becomes -8 |
| `jal item9b` | The `foo`/`bar` loop that adds 2 to each element |
| `jal item10b` | `lui t0,0x32a15`: t0 becomes 0x32a15000 |
| `jal item11b` | `lb t3,-11(s2)` and sign extension |
| `jal item12a` | `sw s3,-8(a1)`, decoded from `0xff35ac23` |
| `jal item13b` | `bne t3,zero,repeat` (offset -8) |

This file has not been run in RARS yet.
