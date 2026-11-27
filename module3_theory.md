# Module 3: Logic Design — the questions that cannot be run

This file holds the true/false statements, the multiple choice question, the explain-in-words
questions and the "construct and draw a circuit" questions of Q3.
Everything that can be computed is in the four C files:

| File | What is in it |
|---|---|
| `module3_combinational.c` | truth tables, Boolean expressions, perfect induction, mux and decoder circuits |
| `module3_sequential.c` | clock-tick tables, enabled D flip-flop, SR latch, counters, register chains |
| `module3_register_file.c` | register file sizes and the register-file circuit |
| `module3_arithmetic.c` | carry propagate adder, two's complement, adder/subtractor |

How to read it:

- Questions are grouped **by exam sitting**. Item numbers are the sittings in date order
  (1 = 2016-06-09 ... 13 = 2020-06-05), the same numbers as in the C files.
- "Key" is the examiner's answer. The motivation is the examiner's, put in plain words.
- "Why" is an extra explanation that is **not** in the key (general course knowledge).
- A figure cannot be shown here, so every circuit is described in words.
- The "extra" questions at the end are screenshots that belong to none of the 13 sittings.
  There is **no official solution** for them in the solution folder.

---

## Words you need first

| Word | Plain meaning |
|---|---|
| **Combinational circuit** | A circuit with no memory. The outputs depend only on the inputs right now. (The exams also write "combinatorial".) |
| **Sequential circuit** | A circuit with memory (a latch, a flip-flop, a register). The outputs depend on the inputs now **and** on what happened before. |
| **D latch** | A 1-bit memory that is **level-sensitive**: while the clock is 1 it is transparent (the output follows the input), while the clock is 0 it holds its value. |
| **D flip-flop** | A 1-bit memory that is **edge-triggered**: it copies D to Q only at the moment the clock goes from 0 to 1 (the rising edge). |
| **Register** | Several flip-flops side by side that share one clock. An N-bit register stores N bits. |
| **Enable (EN)** | An extra input on a flip-flop or register. It only loads a new value on a clock edge if EN = 1. |
| **SR latch** | Two cross-coupled NOR gates. S = set, R = reset. With S = 0 and R = 0 it remembers its old value. |
| **Register file** | The building block that stores the register values of a processor. It has several read ports and one write port. Not the same thing as a single register. |
| **Multiplexer (mux)** | A selector. The select signal picks which ONE of the inputs is passed to the single output. |
| **Decoder** | N inputs, 2^N outputs. Exactly one output is 1: the one whose number equals the input. This is called **one-hot**. |
| **Tristate buffer** | A buffer with an enable signal. When it is not enabled, the output is not driven at all: it is **floating** (written Z). |
| **Carry propagate adder (CPA)** | An adder for two N-bit numbers with a carry in and a carry out. |
| **Perfect induction** | Proving a Boolean rule by trying every combination of inputs in a truth table (also called proof by exhaustion). |
| **Sum-of-products** | A Boolean expression built from a truth table: one AND term for every row where Y = 1, all joined with OR. |
| **Moore machine** | A state machine whose outputs depend only on the current state. |
| **Mealy machine** | A state machine whose outputs depend on the current state **and** on the current inputs. |

---

## Item 1 — 2016-06-09

Source: `2016-06-09 Q3.pdf` (also `Q3x6.png`, `Q3.13.png`)

### Item 1 (a) — construct and draw a circuit (3 p)

**Question.** You have one 4:1 multiplexer, one 2:4 decoder, one 2-bit register, one 2-input AND
gate and one 2-input XOR gate. Draw a combinational circuit with a 2-bit input A and three 1-bit
outputs B, C, D. You do not need to use all the components.

| A | B | C | D |
|---|---|---|---|
| 0 | 0 | 0 | 1 |
| 1 or 2 | 1 | 0 | 0 |
| 3 | 0 | 1 | 0 |

**Key (the examiner's drawing, in words).**

- A (2 bits) goes into the 2:4 decoder.
- **D** = decoder output 00.
- **C** = decoder output 11.
- **B** = the XOR gate, with decoder outputs 01 and 10 as its two inputs.
- The mux, the register and the AND gate are not used.

Marking: one point for each correct output signal.

**Why.** The decoder makes one output high for each value of A. D must be 1 only for A = 0, which
is exactly output 00. C must be 1 only for A = 3, which is exactly output 11. B must be 1 for
A = 1 or A = 2. Outputs 01 and 10 are never 1 at the same time, so XOR works here like an OR.
A register must not be used anyway: with a register the circuit would no longer be combinational.

Runnable check: `item1a` in `module3_combinational.c`.

### Item 1 (b) — sequential or combinational? (2 p)

**Question.** A circuit made of an adder and a register (flip-flop) with a clock. The adder adds
the constant 1 to the register's output, and the sum goes back into the register. Is it a
sequential or a combinational logic circuit? Motivate in at most three sentences.

**Key.** It is a **sequential** logic circuit, because the circuit contains a register, which is a
form of memory.

Marking: 1 point for "sequential", 1 point for a good motivation.

### Item 1 (c) — name the building block (1 p)

**Question.** What is the digital building block called that stores register values in a
processor? It typically has several read ports and one write port.

**Key.** A **register file**.

Marking: 0 points for just "register", because that is not the same thing as a register file.

### Item 1 (d) — D latch vs D flip-flop (2 p)

**Question.** What is the difference between a D latch and a D flip-flop? At most 3 sentences.

**Key.** A flip-flop is **edge-triggered**, whereas a D latch is **level-sensitive**, that is, the
latch is transparent when the clock signal is 1.

Marking: 1 point for "the flip-flop is edge-triggered", 1 point for "the D latch is
level-sensitive".

---

## Item 2 — 2017-01-12

Source: `2017-01-12 Q3.pdf`

Runnable parts: (a) `item2a` in `module3_combinational.c`, (b) `item2b` in `module3_register_file.c`.

### Item 2 (c) — two true/false statements (2 p)

| | Statement | Key | Motivation |
|---|---|---|---|
| i | The difference between a combinatorial circuit and a sequential circuit is that a combinatorial circuit can contain data latches, whereas a sequential circuit can only contain memory elements with floating signal values. | **False** | A combinatorial circuit cannot have any memory, whereas sequential circuits can contain memory building blocks, such as registers. |
| ii | Proof by perfect induction means that you prove the truth by exhaustively showing all possible cases. | **True** | (no motivation needed) |

Marking: one point for each correct answer. A false answer needs a short motivation (2 sentences).

---

## Item 3 — 2017-03-16

Source: `2017-03-16 Q3.pdf`

No theory question. Runnable parts: (a) `item3a` in `module3_sequential.c`, (b) `item3b` in
`module3_combinational.c`, (c) `item3c` in `module3_register_file.c`.

---

## Item 4 — 2017-06-08

Source: `2017-06-08 Q3.pdf`

No theory question. Runnable parts: (a) `item4a` in `module3_register_file.c`, (b) `item4b` in
`module3_arithmetic.c`, (c) `item4c` in `module3_sequential.c`.

---

## Item 5 — 2018-01-15

Source: `2018-01-15 Q3.pdf` (also `Q3.9.png`, `t5.png`)

Runnable parts: (a) `item5a` in `module3_register_file.c`, (b) `item5b` in `module3_arithmetic.c`.

### Item 5 (c) — expression and mux circuit from a truth table (4 p)

**Question.** For the truth table below, (1) write a Boolean algebra expression (it does not have
to be in the simplest form), and (2) draw a circuit for it using one 2:1 multiplexer and one or
two NOT gates.

| X1 | X2 | X3 | Y |
|---|---|---|---|
| 0 | 0 | 0 | 1 |
| 0 | 0 | 1 | 0 |
| 0 | 1 | 0 | 1 |
| 0 | 1 | 1 | 0 |
| 1 | 0 | 0 | 0 |
| 1 | 0 | 1 | 0 |
| 1 | 1 | 0 | 1 |
| 1 | 1 | 1 | 1 |

**Key (1).** Sum-of-products form, where ' means NOT:

`Y = X1'·X2'·X3' + X1'·X2·X3' + X1·X2·X3' + X1·X2·X3`

Marking: 2 points if completely correct. Simplified variants are also accepted.

**Key (2), two possible circuits (the examiner's drawings, in words).**

- **Circuit 1:** the select signal of the mux is **NOT X1**. Mux input 0 is **X2**. Mux input 1 is
  **NOT X3**. The mux output is Y. (Two NOT gates.)
- **Circuit 2:** the select signal of the mux is **X1**. Mux input 0 is **NOT X3**. Mux input 1 is
  **X2**. The mux output is Y. (One NOT gate.)

Marking: 2 points for a completely correct circuit, 1 point if at most one mistake.

**Why.** Look at the table in two halves. In the top half (X1 = 0) the Y column is 1,0,1,0, which
is NOT X3. In the bottom half (X1 = 1) the Y column is 0,0,1,1, which is X2. So X1 chooses
between "NOT X3" and "X2", and choosing is what a mux does.

Runnable check of both parts: `item5c` in `module3_combinational.c`.

---

## Item 6 — 2018-03-15

Source: `2018-03-15 Q3.pdf`

No theory question. Runnable parts: (a) `item6a` in `module3_combinational.c` (the proof by perfect
induction is a truth table, so it can be run), (b) `item6b` in `module3_sequential.c`.

How the proof must look on paper: a truth table with all 8 rows of A, B, C, one column for the
left side and one for the right side, and a sentence saying that the two columns are equal in
every row. Marking: 1 point for a table with 8 cases, 2 points if almost correct (at most 4
errors), 3 points if completely correct.

---

## Item 7 — 2018-06-05

Source: `2018-06-05 Q3.pdf`

No theory question. Runnable parts: (a) `item7a` in `module3_sequential.c`, (b) `item7b` in
`module3_register_file.c`.

---

## Item 8 — 2019-01-12

Source: `2019-01-12-riscv Q3.pdf` (also `Q3.6.png`, `Q3x4.png`)

Runnable part: (a) `item8a` in `module3_register_file.c`.

### Item 8 (b) — construct and draw an adder/subtractor (4 p)

**Question.** Draw a circuit with three inputs A, B, C and one output R. A, B and R are 16-bit
signals in two's complement form. C is one bit. If C = 0 then R = A + B. If C = 1 then
R = A − B. Allowed components: 16-bit carry propagate adder, 2:1 multiplexer, 4:1 multiplexer,
2:4 decoder, inverter, AND gate, OR gate. Not all must be used. Write out the bus widths.

**Key (the examiner's drawing, in words).**

- B goes to **input 0** of a 2:1 mux.
- B also goes through an **inverter** (NOT), and the inverted B goes to **input 1** of the mux.
- **C** is the select signal of the mux.
- A **16-bit carry propagate adder** has A as one input and the mux output as the other input.
- **C is also the carry in** of the adder.
- The adder output is **R**, marked as a 16-bit bus.

Marking: 1 point for the adder with the 16-bit bus. 1 point for the mux with the inverted B.
1 point for C as the carry in. 1 point if the whole circuit works exactly as specified. A circuit
that only adds gives at most 1 point, one that only subtracts at most 2 points.

**Why.** In two's complement, −B is "invert all bits of B, then add 1". So
A − B = A + NOT(B) + 1. When C = 1 the mux picks NOT(B) and the carry in supplies the "+ 1".
When C = 0 the mux picks B and the carry in adds 0, so the result is A + B.

Runnable check: `item8b` in `module3_arithmetic.c`.

---

## Item 9 — 2019-03-15

Source: `2019-03-15-riscv Q3.pdf`

Runnable parts: (a) `item9a` in `module3_sequential.c`, (b) `item9b` in `module3_combinational.c`.

### Item 9 (c) — multiple choice, only one alternative is correct (1 p)

| | Alternative | Correct? |
|---|---|---|
| i | When the enable signal of a tristate buffer is not enabled, the output is said to be floating. | **Yes — this is the key's answer** |
| ii | A multiplexer has a one-hot output signal. That is, for any given input, one and only one output signal is high at the same time on a multiplexer. | No |
| iii | A carry propagate adder (CPA) cannot be used when adding together signed integer values. | No |
| iv | A combinatorial circuit is a circuit that depends on both the current and prior input values. | No |

**Key.** Alternative **i)**. No motivation is asked for.

**Why the others are wrong** (not in the key, general course knowledge):

- ii describes a **decoder**. A mux has only one output.
- iii: the same adder works for signed numbers in two's complement. That is the point of two's
  complement (see item 8 (b), where a CPA does the subtraction).
- iv describes a **sequential** circuit. A combinatorial circuit depends only on the current inputs.

---

## Item 10 — 2019-06-05

Source: `2019-06-05-riscv Q3.pdf`

No theory question. Runnable parts: (a) `item10a` and (b) `item10b` in `module3_combinational.c`,
(c) `item10c` in `module3_register_file.c`.

---

## Item 11 — 2020-01-13

Source: `2020-01-13-riscv Q3.pdf` (also `Q3.3.png`)

Runnable parts: (a) i and ii `item11a` in `module3_sequential.c`, (b) `item11b` in
`module3_register_file.c`, (c) `item11c` in `module3_combinational.c`.

### Item 11 (a) iii — the name of the circuit (1 p)

**Question.** Two NOR gates. The upper one has input A and output C. The lower one has input B and
output D. Each gate's output is the second input of the other gate (cross-coupled). What is the
official name of this circuit?

**Key.** **SR latch.** One point for the explicit answer "SR latch".

For the record, the rest of the key for (a): i) A = 1, B = 0 gives C = 0 and D = 1.
ii) A = 0, B = 0 gives C = unknown and D = unknown (the latch remembers its old value, and we are
not told what that was).

---

## Item 12 — 2020-03-11

Source: `2020-03-11-riscv Q3.pdf`

No theory question. Runnable parts: (a) `item12a` in `module3_combinational.c`, (b) `item12b` in
`module3_register_file.c`, (c) `item12c` in `module3_sequential.c`.

---

## Item 13 — 2020-06-05

Source: `2020-06-05-riscv Q3.pdf`

No theory question. Runnable parts: (a) `item13a` in `module3_combinational.c`, (b) `item13b` and
(c) `item13c` in `module3_register_file.c`.

---

## Extra questions (screenshots that belong to none of the 13 sittings)

There is **no official solution** for these in the solution folder. The answers below are worked
out from general course knowledge, not copied from a key. Two screenshots (`Q3.18.png` and
`Q3.19.png`) do have the solution printed inside the picture; those two are runnable and are in
the C files.

### extra_Q3x7 — Mealy or Moore? (2 p)

Source: `Q3x7.png`

**Question.** A block diagram of a state machine. Is it a Mealy or a Moore machine? Which wire
(A, B or C) should be removed to get the other kind?

The diagram in words:

- The **inputs** (M bits) go into a block called *Next State Logic*.
- The *Next State Logic* output ("next state", k bits) goes into a clocked register. The register
  output is the "state" (k bits).
- Wire **A** takes the state back to the *Next State Logic*.
- Wire **C** takes the state into a block called *Output Logic*, which produces the outputs (N bits).
- Wire **B** takes the **inputs** directly into the *Output Logic*.

**Answer (no official key).** It is a **Mealy machine**, because the output logic gets the inputs
directly (wire B), so the outputs depend on both the state and the inputs. Remove wire **B** and
the outputs depend only on the state, which makes it a **Moore machine**.

Why not the other wires: without A the next state could not depend on the current state, and
without C the outputs could not depend on the state at all.

### extra_t7 — which gate has this truth table? (2 p)

Source: `t7.png`

**Question.** A truth table with inputs A, B, C. Y is 1 in every row except the last one
(A = B = C = 1), where Y is 0. Which logic gate or building block is it?

**Answer (no official key).** A **3-input NAND gate**: Y = NOT(A·B·C).

Runnable check: `extra_t7` in `module3_combinational.c`.

### The other extra questions are runnable

| Screenshot | Function | File |
|---|---|---|
| `Q3.14.png` | `extra_Q3_14` | `module3_sequential.c` |
| `Q3.15.png` | `extra_Q3_15` | `module3_sequential.c` |
| `Q3.16.png` | `extra_Q3_16` | `module3_arithmetic.c` |
| `Q3.17.png` | `extra_Q3_17` | `module3_combinational.c` |
| `Q3.18.png` | `extra_Q3_18` | `module3_sequential.c` |
| `Q3.19.png` | `extra_Q3_19` | `module3_arithmetic.c` |
| `Q3x8.png` | `extra_Q3x8` | `module3_sequential.c` |
| `t8.png` | `extra_t8` | `module3_combinational.c` |
| `t9.png` | `extra_t9` | `module3_combinational.c` |
