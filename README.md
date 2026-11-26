# Computer-Organisation

Runnable master files for IS1500 Computer Organization and Components, one per module.
Each file collects that module's exam questions as code you can compile and run.

| Module | Topic | File | Status |
|---|---|---|---|
| 1 | C and Assembly Programming, C part | `module1_c_programming.c` | in progress |
| 1 | C and Assembly Programming, machine-code part | `module1_machine_code.c` | 13 items |
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
