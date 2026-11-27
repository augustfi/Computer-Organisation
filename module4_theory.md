# Module 4: Processor Design — theory questions

These are the Module 4 exam questions (or parts of questions) that are answered **in words**.
They cannot be run as code. Each one gives the question in short, the examiner's answer, and the
examiner's reason. Where a part of the question *can* be run, the C item is named.

- Runnable datapath items: `module4_datapath.c`
- Runnable ALU items: `module4_alu.c`
- Runnable hazard items: `module4_hazards.c`

Items 1–7 and the "extra" items come from the MIPS exams (2016–2018). The exam is RISC-V now.
The ideas (single-cycle datapath, pipeline, hazards) are the same. What differs is written at each item.

---

## Words you must know

| Word | Meaning |
|---|---|
| Single-cycle processor | Every instruction takes exactly 1 clock cycle (CPI = 1). The clock must be slow enough for the slowest instruction. |
| Critical path | The longest way a signal must travel in one clock cycle. It decides how fast the clock can be. |
| Pipeline (5 stages) | Fetch, Decode, Execute, Memory, Writeback. Each stage is short, so the clock can be faster. |
| Data hazard | An instruction needs a register that an earlier instruction has not written back yet. Also called read-after-write (RAW) hazard. |
| Forwarding | Send the value straight from a later stage to where it is needed. No extra cycle. |
| Stalling | Hold an instruction back one or more cycles. Costs cycles. |
| Control hazard | After a branch, the next instructions are fetched before the processor knows if the branch is taken. |
| Flushing | Throw away the wrongly fetched instructions when a branch is taken. |
| Predict not taken | The processor guesses that a branch is not taken and keeps fetching the next lines. |
| Branch delay slot | Old MIPS rule: the instruction right after a branch is always executed, taken or not. RISC-V does not have this, but two of the rewritten RISC-V exams still assume it — read the exam text. |

One rule appears in many answers: **the register file writes in the first half of a clock cycle and
reads in the second half.** So an instruction that is 3 lines after the writer reads the new value
without any help. Only 1 or 2 lines apart is a data hazard.

---

## Item 1 — 2016-06-09 (MIPS era)

Source: `2016-06-09 Q4.pdf`

**(a)** For each ALU function, give an example instruction. Run: `item1a` in `module4_alu.c`.
Examiner: F = 1 is OR (an `or`), F = 6 is subtract (a `sub`), F = 7 is set-less-than (a `slt`).
In RISC-V the instruction names `or`, `sub`, `slt` are the same.

**(b)** ALU signals for `sb $t0,-5($s0)`. Run: `item1b` in `module4_alu.c`.
Examiner's reasons:
- The ALU does an **add** (F = 2), because a store must add the offset to the base register to get the address.
- A is the **value** of the register (0x11118888), not the register number.
- The exam did not say clearly that the ALU is 32 bits. So the 8-bit answer (A = 0x88, Y = 0x83) is also counted as correct.

**(c)** `lb $t0,8($s0)` then `lw $s0,8($t0)`: which register, best solution? Run: `item1c` in `module4_hazards.c`.
Examiner: register `$t0`, solved with **forwarding and stalling** together.
Reason: `lb` is a load. Its value only exists after the memory stage, but the next instruction needs it
one stage earlier (in execute). Forwarding alone cannot send a value back in time, so one stall is needed first.

---

## Item 2 — 2017-01-12 (MIPS era)

Source: `2017-01-12 Q4.pdf`

**(a)** Find the instruction from the signal values. Run: `item2a` in `module4_datapath.c`.
Answer: `ori $a0,$s4,0x122`. (The exam's footnote says the original exam had a typo in signal D.)

**(b)** `addi $t0,$s0,1` / `lw $t0,-8($s0)` / `sw $t1,-4($t0)` / `or $t0,$s0,$t0`: which hazards?
Run: `item2b` in `module4_hazards.c`.
Examiner: **two different answers are accepted**:
1. Only one data hazard: between `lw` and `sw`, register `$t0`, solved with stalling **and** forwarding.
2. Read-after-write hazards at both `sw` and `or`. Both use `$t0` before it is written to the register.
   The first is solved with stalling and forwarding.

Examiner's note: if `sw` is stalled one cycle, `or` also comes one cycle later. Then `lw` has already
written `$t0` (write in the first half of the cycle, read in the second half). So after the stall there
is really no hazard left for `or`. That is why answer 1 is right too.

---

## Item 3 — 2017-03-16 (MIPS era)

Source: `2017-03-16 Q4.pdf`

**(a)** `addi` / `lw` / `bne` / `ori` / `andi` on a pipeline **with branch delay slots**.
Run: `item3a` in `module4_hazards.c`.
- i. Kind of hazard: **control hazard**. No register that is read was written by a line just before, so there is no data hazard.
- ii. `$s0` afterwards: **0x4**. With delay slots the instruction after the branch (`ori`) always runs, taken or not.
- iii. Forwarding used: **no**. There is no data hazard, so nothing to forward.

In RISC-V there are no delay slots. Without them, a taken branch would skip `ori`, and `$s0` could be
either 0x4 or its old value — the answer would be "unknown".

**(b)** Signals for `lb $t3,-7($s0)`. Run: `item3b` in `module4_datapath.c`.
Examiner's note in words: the figure can do `lw`, but only *partly* `lb`. A real load-byte must also
sign extend the lowest 8 bits of the word it reads. The figure has no part for that. It does not change
the answer here, because the value read from memory is unknown anyway.

---

## Item 4 — 2017-06-08 (MIPS era)

Source: `2017-06-08 Q4.pdf`

**(a)** Run: `item4a` in `module4_datapath.c`.
Point to remember: signal E (the branch target) has a value although `addi` is not a branch.
Every part of the datapath calculates in every cycle; the control signals only decide what is *used*.

**(b)** Choose the missing instruction between `lw $s1,20($t0)` and `xor $s2,$t0,$t3`.
Run: `item4b` in `module4_hazards.c`.
Examiner: `addi $t3,$s1,0` (several answers are correct).
- It reads `$s1`, which `lw` writes: hazard solved with **forwarding and stalling** (a load).
- It writes `$t3`, which `xor` reads: hazard solved with **forwarding only**.

---

## Item 5 — 2018-01-15 (MIPS era)

Source: `2018-01-15 Q4.pdf`

**(b)** Run: `item5b` in `module4_hazards.c`.
Examiner: lines 1–2 on `$t1`, lines 2–3 on `$t2`, **forwarding in both cases**.
Worth noticing (not said by the examiner, but it follows from the answer): lines 2–3 are a load
followed by a store of the same register. No stall is needed, because `sw` only needs `$t2` as the
value to store, and that is used one stage later (memory) than an address would be (execute).

---

## Item 6 — 2018-03-15 (MIPS era)

Source: `2018-03-15 Q4.pdf`

**(a)** `addi $t1,$zero,5` / `foo: lw` / `sw` / `beq $t1,$zero,foo`: how many hazards?
Run: `item6a` in `module4_hazards.c`.
Examiner: **zero control hazards, one data hazard, only register `$t0`**.
- `$t1` is 5, so `beq $t1,$zero` will never jump. A branch that never jumps gives no control hazard.
- No data hazard between `addi` and `beq`: `addi` writes the register in the first half of the same
  cycle in which `beq` reads it.
- The data hazard is between `lw` and `sw`.
- The answer assumes the comparison is done in the execute stage, but it is the same if it is done in decode.

---

## Item 7 — 2018-06-05 (MIPS era)

Source: `2018-06-05 Q4.pdf`

**(b)** Run: `item7b` in `module4_hazards.c`.
Examiner: only one data hazard, between `slt` and `beq`, register `$t1`. Solved by **stalling one
clock cycle and then forwarding**.
Reason: the comparison is in the decode stage, so `beq` needs `$t1` one stage earlier than a normal
instruction. `slt` has not finished its execute stage yet.

---

## Item 8 — 2019-01-12 (RISC-V)

Source: `2019-01-12-riscv Q4.pdf`

**(a)** Run: `item8a` in `module4_datapath.c`.
**Warning: the official answer for signal E looks wrong.** The key says E = 0x4000. Following the
program step by step gives 0x400 (0x3f0 + 16 = 0x400 is stored and then loaded back). The key's own
value for B (0x3f8, the address) fits 0x400. The exam was rewritten from MIPS in 2025 and the numbers
in the question seem to have been made smaller without updating E in the key.

**(b)** `lw t0,0x708(zero)` / `beq t0,zero,foo` / `addi` / `foo: addi`, comparison in decode, predict not taken.
Run: `item8b` in `module4_hazards.c`.
- i. One data hazard and one control hazard.
- ii. The `lw` and `beq` instructions.
- iii. **3 clock cycles**: two stall cycles before the memory result of `lw` can be forwarded to the
  decode stage of `beq`, plus one cycle of penalty if the branch is taken.

---

## Item 9 — 2019-03-15 (RISC-V)

Source: `2019-03-15-riscv Q4.pdf`

**(a)** Run: `item9a` in `module4_datapath.c`.
Slip in the question: the text says "single-cycle 32-bit **MIPS** processor". The figure and the
registers are RISC-V. Answer it as RISC-V.

**(b)** Run: `item9b` in `module4_hazards.c`.
Examiner: two hazards. `addi` → `sll` on `t0`, and `sll` → `lw` on `t2`. Both solved with forwarding.

---

## Item 10 — 2019-06-05 (RISC-V)

Source: `2019-06-05-riscv Q4.pdf`

**(b)** Run: `item10b` in `module4_hazards.c`. The exam says: branch delay slots, comparison in decode.
- i. The data hazard is between `lw` and `beq`.
- ii. Yes, stalling is needed: **2 clock cycles**.
- iii. `t0` is **5**. The branch delay slot executes the `addi t0,zero,5` after the `beq`, also when the branch is taken.

---

## Item 11 — 2020-01-13 (RISC-V)

Source: `2020-01-13-riscv Q4.pdf`

**(a)** Run: `item11a` in `module4_datapath.c`.
Slip in the question: it says "Suppose the **bne** instruction is executing" once, and `beq`
everywhere else. The official answer (branch taken, C = 1) fits `beq`.
Idea to remember: `xor t1,t2,t2` gives 0 whatever `t2` is. So `t1` is known even though `t2` is unknown.

**(b)** Run: `item11b` in `module4_hazards.c`.
Examiner: data hazards 2-4, 3-4, 5-7. Only 3-4 (load, then use) needs stalling.

---

## Item 12 — 2020-03-11 (RISC-V)

Source: `2020-03-11-riscv Q4.pdf`

**(a)** Run: `item12a` in `module4_hazards.c`.
Examiner: only one data hazard, between `addi` and `sw`, register `s1`, solved with forwarding.

**(b)** Run: `item12b` in `module4_datapath.c`.
Trap: `sw t0,8(s1)` and `lw s2,0(t1)` with `t1 = s1` look like "store, then load the same word".
They are not: the offsets are 8 and 0. So what `lw` reads is unknown.

---

## Item 13 — 2020-06-05 (RISC-V)

Source: `2020-06-05-riscv Q4.pdf`

**(a)** Run: `item13a` in `module4_datapath.c`.
Slip in the question: `lw t8,4(t2)` — RISC-V has no register `t8`. It does not change any of the signals asked for.

**(b)** Run: `item13b` in `module4_hazards.c`.
Examiner: **zero control hazards and zero data hazards**.
Reason: the branch is never taken (no control hazard), and the value of `t1` is already available
in the pipeline when `beq` reads it (no data hazard).

---

## Extra questions (PNG only, not from the 13 sittings — all MIPS era)

### extra_Q4_7

Source: `Q4-Q7.png`

**(b)** `add $t0,$t1,$t2` / `lw $t1,-4($t2)` / `beq $t2,$zero,skip` / `addi $t0,$zero,1`.
Run: `extra_Q4_7b` in `module4_hazards.c`.
Examiner:
- Only the `beq` instruction can cause a hazard.
- It is a **control hazard**, if the branch is taken.
- It is handled by **flushing** the pipeline and fetching a new instruction.

Examiner's extra note: there is also a write-after-read (WAR) hazard (`lw` writes `$t1` after `add`
read it). That only matters in an out-of-order processor, which can change the order of
instructions. It does not matter in this 5-stage pipeline.

### extra_Q4_8

Source: `Q4-Q8.png`

**(b)** Run: `extra_Q4_8b` in `module4_hazards.c`.
Examiner: the missing instruction must be a load word (or load byte) that writes to `$t0`,
for example `lw $t0,0($t1)`. The base register can be any register.

### extra_Q4_10

Source: `Q4-Q10.png` (its header says "Module 3", the numbering of an older year)

**(a)** Run: `extra_Q4_10a` in `module4_datapath.c`.

**(b)** In a single-cycle processor each instruction takes 1 clock cycle. Why can a 5-stage
pipelined processor still be faster? (At most two sentences.)

Examiner: because the **critical path for each clock cycle is shorter**, so the processor can be
**clocked at a higher clock frequency**.
Examiner's extra note: the cycles per instruction (CPI) can actually be *larger* for a pipelined
processor than for a single-cycle processor (because of stalls and flushes). The gain comes from the
shorter clock period.

In formula form (lecture 10): execution time = number of instructions × CPI × clock period.
Single-cycle: CPI = 1, but a long clock period. Pipeline: CPI a little above 1, but a much shorter clock period.

**(c)** The processor uses "branch not taken". Which kind of hazard can `beq` cause, and what is the
earliest stage where the misprediction can be detected, if the penalty is 1 clock cycle?
Run: `extra_Q4_10c` in `module4_hazards.c`.

Examiner: a **control hazard**, and the misprediction can be detected in the **decode stage**.
Reason: when the branch is in decode, only one wrong instruction has been fetched after it. One
instruction to flush = 1 cycle of penalty.

### extra_Q4_11

Source: `Q4-Q11.png` (header says "Module 3")

**(a)** `addi $s0,$t0,8` / `sw $t0,8($s0)` / `sub $s0,$s0,$t1`. Run: `extra_Q4_11a` in `module4_hazards.c`.
Examiner (at most 4 sentences):
- i) The `addi`, the `sw` and the `sub` cause hazards, because the result in `$s0` is not in the register file when `sw` and `sub` need it.
- ii) These are **data hazards**.
- iii) Solved by **forwarding** (for example: `$s0` is computed in the execute stage of `addi` and forwarded to the execute stage of `sw`, which computes the address).
- iv) Forwarding gives **no extra cost** in clock cycles.

**(b)** Run: `extra_Q4_11b` in `module4_datapath.c`.

### extra_Q4_12

Source: `Q4-Q12.png`. Run: `extra_Q4_12a` in `module4_datapath.c`.

### extra_Q4_13 — four true/false statements (single-cycle MIPS datapath)

Source: `Q4-Q13.png`

| # | Statement | Examiner | Examiner's reason |
|---|---|---|---|
| 1 | If the instruction is `lw $t2,8($s2)`, the control unit sets ALUSrc to 1. | **True** | `lw` is an I-type instruction and the ALU computes the address. The second ALU input comes from the sign-extended immediate. |
| 2 | For `j foo`, the 16 lowest bits of the instruction are sign extended to 32 bits to form the jump address, and the Branch control signal is 1. | **False** | Several parts are wrong. The address bits are not only sign extended: they are shifted and completed with bits from the PC. And it is the **Jump** signal that is 1, not Branch. |
| 3 | For `add $t4,$s0,$t2`, the 5-bit input A3 of the register file has the value 10. | **False** | `add` is R-type. A3 is the number of the destination register. That is `$t4`, which is number 12, not 10. |
| 4 | For an R-type instruction that writes to the register file, MemToReg must be 0, so that the ALU result goes around the data memory. | **True** | (No reason given by the examiner. MemToReg = 0 picks the ALU result at the last mux.) |

How these would look in RISC-V:
1. Same: `lw` sets ALUSrc = 1.
2. RISC-V jumps with `jal`. The simple RISC-V figure used in the 2019–2020 exams has no jump path at all.
3. For `add t4,s0,t2` A3 = Instr[11:7] = 29 (`t4` is register 29 in RISC-V).
4. Same, but the signal is called ResultSrc.
