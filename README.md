# Computer-Organisation

Runnable master files for IS1500 Computer Organization and Components, one per module.
Each file collects that module's exam questions as code you can compile and run.

| Module | File | Covers |
|---|---|---|
| 1 C and Assembly Programming | `module1_c_programming.c` | Pointer and array C questions |
| 1 | `module1_machine_code.c` | Encode and decode RISC-V machine code |
| 1 | `module1_assembly.asm` | The same items as RISC-V assembly |
| 2 I/O Systems | `module2_timer.c` | Timer questions: period register, prescale, software counter |
| 2 | `module2_io.c` | C side of memory-mapped I/O: `volatile`, masks and shifts |
| 2 | `module2_io.asm` | LEDs, switches, buttons, 7-segment display, polling loops in RISC-V |
| 2 | `module2_theory.md` | True/false and explain questions |
| 3 Logic Design | `module3_combinational.c` | Truth tables, Boolean expressions, perfect induction, mux and decoder circuits |
| 3 | `module3_sequential.c` | Clock-tick tables, flip-flops, SR latch, counters |
| 3 | `module3_register_file.c` | Register file size, address and data widths, circuit signals |
| 3 | `module3_arithmetic.c` | Adder with carry, two's complement, adder/subtractor |
| 3 | `module3_theory.md` | Explain, true/false and draw-a-circuit questions |
| 4 Processor Design | `module4_datapath.c` | Signals A to E on the single-cycle datapath |
| 4 | `module4_alu.c` | ALU output for a function code |
| 4 | `module4_hazards.c` | Data and control hazards on the 5-stage pipeline |
| 4 | `module4_theory.md` | True/false and explain questions |
| 5 Memory Hierarchy | `module5_cache_fields.c` | Tag, set and offset bits, valid bits, capacity, associativity |
| 5 | `module5_cache_sim.c` | Hit and miss rates through a small cache simulator |
| 5 | `module5_theory.md` | True/false and explain questions |
| 6 Parallel Processors and Programs | `module6_amdahl.c` | Amdahl's law in all variants |
| 6 | `module6_misc.c` | CPI on a multiple-issue processor, operations per AVX instruction |
| 6 | `module6_theory.md` | True/false statements by sitting and by topic |

## How the files work

Every exam item is its own function. Items are numbered by exam sitting in date order
(1 = 2016-06-09 ... 13 = 2020-06-05) with the sub-question letter after it, for example
`item9a`. Questions that exist only as a screenshot are named `extra_...`.

Each C item computes its answer and prints it beside the exam's answer ("key") with `OK`
or `WRONG`. Run any C file like this (replace the name):

```
gcc module5_cache_sim.c -o run && ./run
```

`main()` calls every item. Comment out calls to run fewer. (`module1_c_programming.c` is
the exception: it runs one item at a time.)

The `.asm` files need RARS, a RISC-V simulator. They have not been run in RARS yet.

Things to know:

- The 2016 to 2018 exams were MIPS. Their assembly is redone in RISC-V in modules 1 and 2;
  those RISC-V versions are this repo's own, not the examiner's. Module 4 keeps those
  items in MIPS so they can be checked against the official key.
- Some screenshot-only questions have no official solution. Their output says "no key".
- Two official solutions look wrong and are flagged in the files: the 2020-03-11 I/O
  program (module 2, item 12b) and signal E of the 2019-01-12 datapath question
  (module 4, item 8a).

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
