#include <stdio.h>

// =====================================================================
// Module 3 (exam question 3): LOGIC DESIGN, part 2: SEQUENTIAL circuits
//
// A sequential circuit has memory: a register (flip-flops) that only changes
// on a clock tick, or a feedback wire that makes gates remember (a latch).
//
// How a clock tick is simulated here:
//   1. compute what stands on the INPUT of every register (using the old values)
//   2. copy those inputs into the registers (this is the tick)
//   3. compute all other signals again from the new register values
//
// How a feedback wire (no clock) is simulated here:
//   guess the fed-back signal (first 0, then 1), compute the gates again and
//   again until nothing changes. If both guesses end in the same value, that
//   is the answer. If they end differently, the circuit REMEMBERS its old
//   value, so the answer is "unknown" / "previous value".
//
// Item numbers = exam sittings in date order (see module3_combinational.c).
// "extra_..." items are screenshots that belong to none of the sittings.
//
// Run:  gcc -Wall module3_sequential.c -o seq && ./seq
// =====================================================================


// ---------- The building blocks (same as in module3_combinational.c) ----------

int AND(int a, int b)  { return a & b; }
int OR(int a, int b)   { return a | b; }
int NOT(int a)         { return 1 - a; }
int NAND(int a, int b) { return NOT(AND(a, b)); }
int NOR(int a, int b)  { return NOT(OR(a, b)); }
int XOR(int a, int b)  { return a ^ b; }

// 2:1 mux: s = 0 gives d0, s = 1 gives d1.
int mux2(int d0, int d1, int s) {
    if (s == 0) {
        return d0;
    }
    return d1;
}

// 4:1 mux: the 2-bit select s (0..3) picks one of four inputs.
int mux4(int d0, int d1, int d2, int d3, int s) {
    if (s == 0) return d0;
    if (s == 1) return d1;
    if (s == 2) return d2;
    return d3;
}

// 2:4 decoder: only the output whose number equals the input is 1.
void decoder(int a, int y[4]) {
    for (int i = 0; i < 4; i++) {
        if (i == a) {
            y[i] = 1;
        } else {
            y[i] = 0;
        }
    }
}

// An adder whose result is only "bits" wide: the carry out is thrown away.
// Example with 2 bits: 3 + 1 = 4 = 100 in binary, only 00 is kept, so 0.
int addBits(int a, int b, int bits) {
    int mask = (1 << bits) - 1;      // 2 bits -> mask 11 (3), 3 bits -> 111 (7)
    return (a + b) & mask;
}

// Two cross-coupled NOR gates (an SR latch):
//     top    = NOR(topIn,    bottom)
//     bottom = NOR(bottomIn, top)
// "start" is our guess for the old value of the bottom output.
// The result is stored as out[0] = top output, out[1] = bottom output.
void norLatch(int topIn, int bottomIn, int start, int out[2]) {
    int top = 0;
    int bottom = start;
    // Compute the two gates a few times, so the values can settle.
    for (int round = 0; round < 4; round++) {
        top = NOR(topIn, bottom);
        bottom = NOR(bottomIn, top);
    }
    out[0] = top;
    out[1] = bottom;
}


// ---------- Helpers for printing and checking ----------

void okOrWrong(int ok) {
    if (ok) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Print one row of a tick table and compare it with the key row.
void tickRow(int tick, int a, int b, int c, int d, int key[4]) {
    printf("   %d   | %d %d %d %d | %d %d %d %d  ", tick, a, b, c, d, key[0], key[1], key[2], key[3]);
    okOrWrong(a == key[0] && b == key[1] && c == key[2] && d == key[3]);
}


// ===================== Item 3 (a) =====================
// Source: 2017-03-16 Q3.pdf (also Q3.11.png)
// Question: fill in U for four rows of A, B, C. Write "Upre" if the circuit
//   remembers its previous value.
// Circuit in words (there is a feedback wire, no clock):
//   - NOR gate 1: inputs A and the feedback wire F. Its output is U.
//   - AND gate:   inputs U and B.
//   - NOR gate 2: inputs the AND output and C. Its output is F,
//                 which goes back to NOR gate 1.
// Key:  A B C | U
//       0 0 0 | 0
//       0 0 1 | 1
//       0 1 0 | Upre
//       1 1 0 | 0

// Let the circuit settle, starting from a guessed old value of U.
int settleItem3a(int a, int b, int c, int startU) {
    int u = startU;
    for (int round = 0; round < 4; round++) {
        int andOut = AND(u, b);
        int f = NOR(andOut, c);
        u = NOR(a, f);
    }
    return u;
}

void item3a(void) {
    printf("\nItem 3(a): NOR - AND - NOR with a feedback wire\n");
    int rowA[4] = {0, 0, 0, 1};
    int rowB[4] = {0, 0, 1, 1};
    int rowC[4] = {0, 1, 0, 0};
    // The key. We write 2 for "Upre" (remembers).
    int key[4] = {0, 1, 2, 0};

    printf("  A B C | U     | key\n");
    for (int i = 0; i < 4; i++) {
        int u0 = settleItem3a(rowA[i], rowB[i], rowC[i], 0);   // old U guessed as 0
        int u1 = settleItem3a(rowA[i], rowB[i], rowC[i], 1);   // old U guessed as 1
        int got;
        if (u0 == u1) {
            got = u0;        // the old value does not matter: a fixed answer
        } else {
            got = 2;         // the answer follows the old value: it remembers
        }

        printf("  %d %d %d | ", rowA[i], rowB[i], rowC[i]);
        if (got == 2) printf("Upre  | "); else printf("%d     | ", got);
        if (key[i] == 2) printf("Upre  "); else printf("%d     ", key[i]);
        okOrWrong(got == key[i]);
    }
    printf("  Why row 3 remembers: with A = 0, B = 1, C = 0 we get F = NOT U,\n");
    printf("  and U = NOT F, so U just keeps the value it already had.\n");
}

// ===================== Item 4 (c) =====================
// Source: 2017-06-08 Q3.pdf (also Q3.10.png)
// Question: fill in Y1 and Y0 for four rows of A1 A0 B C1 C0 (after the
//   circuit has stabilized).
// Circuit in words:
//   - a 2:4 decoder with the 2-bit input A
//   - a 4:1 mux: its data inputs 00,01,10,11 are the decoder outputs 00,01,10,11,
//     its select is the 2-bit signal C.  So the mux output M is 1 only if A = C.
//   - two cross-coupled NOR gates:  Y1 = NOR(B, Y0)   Y0 = NOR(M, Y1)
// Key:  A1 A0 B C1 C0 | Y1 Y0
//        0  0 1  1  1 |  0  1
//        0  1 1  1  0 |  0  1
//        1  0 0  1  0 |  1  0
//        1  1 1  1  1 |  0  0
void item4c(void) {
    printf("\nItem 4(c): decoder + mux + cross-coupled NOR gates\n");
    int rowA[4] = {0, 1, 2, 3};
    int rowB[4] = {1, 1, 0, 1};
    int rowC[4] = {3, 2, 2, 3};
    int keyY1[4] = {0, 0, 1, 0};
    int keyY0[4] = {1, 1, 0, 0};

    printf("  A1 A0 B C1 C0 | Y1 Y0 | key\n");
    for (int i = 0; i < 4; i++) {
        int dec[4];
        decoder(rowA[i], dec);
        int m = mux4(dec[0], dec[1], dec[2], dec[3], rowC[i]);

        // Try both old values of Y0. Here they always end the same,
        // because in every row at least one of B and M is 1.
        int try0[2];
        int try1[2];
        norLatch(rowB[i], m, 0, try0);
        norLatch(rowB[i], m, 1, try1);

        printf("   %d  %d %d  %d  %d |", (rowA[i] >> 1) & 1, rowA[i] & 1, rowB[i],
               (rowC[i] >> 1) & 1, rowC[i] & 1);
        if (try0[0] == try1[0] && try0[1] == try1[1]) {
            printf("  %d  %d | %d %d  ", try0[0], try0[1], keyY1[i], keyY0[i]);
            okOrWrong(try0[0] == keyY1[i] && try0[1] == keyY0[i]);
        } else {
            printf(" remembers | %d %d  WRONG\n", keyY1[i], keyY0[i]);
        }
    }
}

// ===================== Item 6 (b) =====================
// Source: 2018-03-15 Q3.pdf (also Q3.8.png)
// Question: fill in A, B, C, D after clock ticks 1 to 4 (decimal numbers).
//   Given: after tick 1, A = 1 and B = 2.
// Circuit in words:
//   - a 2-bit adder: constant 1 + A. The adder output is signal B.
//   - a 2-bit register: its input is B, its output is A (so A becomes B on a tick)
//   - a 4:1 mux with constant inputs 00->0, 01->0, 10->1, 11->1 and select B.
//     Its output is signal C.
//   - a 2:4 decoder: low input bit (pin 0) = constant 1, high input bit (pin 1) = C
//   - D = NOR of decoder outputs 00 and 01
// Key:  Tick | A B C D
//        1   | 1 2 1 1
//        2   | 2 3 1 1
//        3   | 3 0 0 0
//        4   | 0 1 0 0
void item6b(void) {
    printf("\nItem 6(b): counter + mux + decoder + NOR, ticks 1 to 4\n");
    int key[4][4] = {{1, 2, 1, 1}, {2, 3, 1, 1}, {3, 0, 0, 0}, {0, 1, 0, 0}};

    // A is 1 after tick 1 and it goes up by 1 each tick, so the register starts at 0.
    int a = 0;
    int b = addBits(1, a, 2);            // adder output before the first tick

    printf("  Tick | A B C D | key\n");
    for (int tick = 1; tick <= 4; tick++) {
        a = b;                           // the tick: the register takes its input

        b = addBits(1, a, 2);            // then everything else settles
        int c = mux4(0, 0, 1, 1, b);
        int dec[4];
        decoder(c * 2 + 1, dec);         // C is the high bit, constant 1 the low bit
        int d = NOR(dec[0], dec[1]);

        tickRow(tick, a, b, c, d, key[tick - 1]);
    }
}

// ===================== Item 7 (a) =====================
// Source: 2018-06-05 Q3.pdf (also Q3.7.png)
// Question: fill in A, B, C, D after clock ticks 1 to 4 (decimal numbers).
//   Given: after tick 1, A = 2 and B = 3.
// Circuit in words:
//   - a 2-bit adder: constant 1 + A
//   - register 1: input = adder output, output = A
//   - register 2: input = A, output = B   (B is A, one tick late)
//   - a 2:4 decoder with input A
//   - a 4:1 mux: data inputs 00..11 = decoder outputs 00..11, select = B.
//     Its output is D.  So D = 1 only if A = B.
//   - C = NOR of decoder output 00 and D
// Key:  Tick | A B C D
//        1   | 2 3 1 0
//        2   | 3 2 1 0
//        3   | 0 3 0 0
//        4   | 1 0 1 0
// Note: we do not know what the registers held before tick 1, so we start
// from the given values after tick 1 (A = 2, B = 3).
void item7a(void) {
    printf("\nItem 7(a): adder + two registers + decoder + mux, ticks 1 to 4\n");
    int key[4][4] = {{2, 3, 1, 0}, {3, 2, 1, 0}, {0, 3, 0, 0}, {1, 0, 1, 0}};
    int a = 2;                           // given after tick 1
    int b = 3;                           // given after tick 1

    printf("  Tick | A B C D | key\n");
    for (int tick = 1; tick <= 4; tick++) {
        if (tick > 1) {
            // The tick: both registers take their inputs AT THE SAME TIME,
            // so compute both inputs from the old values first.
            int nextA = addBits(1, a, 2);
            int nextB = a;
            a = nextA;
            b = nextB;
        }

        int dec[4];
        decoder(a, dec);
        int d = mux4(dec[0], dec[1], dec[2], dec[3], b);
        int c = NOR(dec[0], d);

        tickRow(tick, a, b, c, d, key[tick - 1]);
    }
}

// ===================== Item 9 (a) =====================
// Source: 2019-03-15-riscv Q3.pdf (also Q3.5.png)
// Question: fill in Q, D, Y.  Row "init": A = 1, B = 0, Q = 0 (before a tick).
//   Row "1st": the values after the first clock tick, with A = 1, B = 1.
// Circuit in words:
//   - a D flip-flop with enable. EN is tied to 1, so it loads D on every tick.
//   - D = XOR(A, Q)      (Q is fed back to the XOR gate)
//   - Y = NAND(Q, B)
// Key:  clock A B | Q D Y
//       init  1 0 | 0 1 1
//       1st   1 1 | 1 0 0
void item9a(void) {
    printf("\nItem 9(a): XOR -> enabled D flip-flop -> NAND\n");
    int en = 1;

    // init row
    int a = 1, b = 0, q = 0;
    int d = XOR(a, q);
    int y = NAND(q, b);
    printf("  clock A B | Q D Y | key\n");
    printf("  init  %d %d | %d %d %d | 0 1 1  ", a, b, q, d, y);
    okOrWrong(q == 0 && d == 1 && y == 1);

    // the clock tick: Q takes the value that D had just before the tick
    if (en == 1) {
        q = d;
    }
    // new inputs, then D and Y settle
    a = 1;
    b = 1;
    d = XOR(a, q);
    y = NAND(q, b);
    printf("  1st   %d %d | %d %d %d | 1 0 0  ", a, b, q, d, y);
    okOrWrong(q == 1 && d == 0 && y == 0);
}

// ===================== Item 11 (a) =====================
// Source: 2020-01-13-riscv Q3.pdf (also Q3.3.png)
// Question: two cross-coupled NOR gates, inputs A and B, outputs C and D.
//     C = NOR(A, D)      D = NOR(B, C)
//   (i)   A = 1, B = 0: what are C and D?
//   (ii)  A = 0, B = 0: what are C and D?   (answer 0, 1 or unknown)
//   (iii) the official name of the circuit: see module3_theory.md (SR latch)
// Key: (i) C = 0, D = 1.   (ii) C = unknown, D = unknown.

// Settle the latch for both guesses of the old value and print the result.
// Returns 1 if the result is a fixed value, 0 if it is unknown (remembers).
int showLatch(int a, int b, int out[2]) {
    int try0[2];
    int try1[2];
    norLatch(a, b, 0, try0);     // old D guessed as 0
    norLatch(a, b, 1, try1);     // old D guessed as 1
    printf("  A = %d, B = %d:  old D = 0 gives C = %d, D = %d.  old D = 1 gives C = %d, D = %d.\n",
           a, b, try0[0], try0[1], try1[0], try1[1]);
    out[0] = try0[0];
    out[1] = try0[1];
    if (try0[0] == try1[0] && try0[1] == try1[1]) {
        return 1;
    }
    return 0;
}

void item11a(void) {
    printf("\nItem 11(a): cross-coupled NOR gates (SR latch)\n");
    int out[2];

    int known = showLatch(1, 0, out);
    if (known) {
        printf("  (i)  C = %d, D = %d   key: C = 0, D = 1   ", out[0], out[1]);
        okOrWrong(out[0] == 0 && out[1] == 1);
    } else {
        printf("  (i)  unknown   key: C = 0, D = 1   WRONG\n");
    }

    known = showLatch(0, 0, out);
    if (known) {
        printf("  (ii) C = %d, D = %d   key: unknown, unknown   WRONG\n", out[0], out[1]);
    } else {
        printf("  (ii) C = unknown, D = unknown   key: unknown, unknown   OK\n");
        printf("       (with A = 0 and B = 0 the latch keeps its old value, which we are not told)\n");
    }
}

// ===================== Item 12 (c) =====================
// Source: 2020-03-11-riscv Q3.pdf (also Q3.2.png)
// Question: fill in Q, D, Y.  Row "init": A = 1, B = 1, Q = 1 (before a tick).
//   Row "1st": the values after the first clock tick, with A = 0, B = 0.
// Circuit in words:
//   - a D flip-flop with enable. EN is tied to 1, so it loads D on every tick.
//   - D = AND(Q, A)      (Q is fed back to the AND gate)
//   - Y = XOR(Q, B)
// Key:  clock A B | Q D Y
//       init  1 1 | 1 1 0
//       1st   0 0 | 1 0 1
void item12c(void) {
    printf("\nItem 12(c): AND -> enabled D flip-flop -> XOR\n");
    int en = 1;

    // init row
    int a = 1, b = 1, q = 1;
    int d = AND(q, a);
    int y = XOR(q, b);
    printf("  clock A B | Q D Y | key\n");
    printf("  init  %d %d | %d %d %d | 1 1 0  ", a, b, q, d, y);
    okOrWrong(q == 1 && d == 1 && y == 0);

    // the clock tick: Q takes the value that D had just before the tick
    if (en == 1) {
        q = d;
    }
    // new inputs, then D and Y settle
    a = 0;
    b = 0;
    d = AND(q, a);
    y = XOR(q, b);
    printf("  1st   %d %d | %d %d %d | 1 0 1  ", a, b, q, d, y);
    okOrWrong(q == 1 && d == 0 && y == 1);
}

// ===================== extra_Q3_14 =====================
// Source: Q3.14.png
// Question: the register starts at 1. The carry in C is either 1 or 0.
//   Which values can signal A have had during 1000 clock ticks, in each case?
// Circuit in words:
//   - a 3-bit register, output A
//   - a 3:8 decoder with input A. Only its output 101 is used:
//     it is the select of a 2:1 mux with constant inputs 0 -> 2 and 1 -> 4.
//   - a 3-bit adder: A + mux output + carry in C. Its output goes to the register.
//   So each tick A grows by 2 (or by 4 when A is 5), plus C, and wraps at 8.
// Key: none in the solution folder. Computed here:
//   C = 0: A is one of 1, 3, 5.      C = 1: A is one of 1, 2, 4, 5, 7.
void extra_Q3_14(void) {
    printf("\nextra_Q3_14: 3-bit counter, values of A during 1000 ticks\n");
    for (int c = 0; c <= 1; c++) {
        int seen[8] = {0};               // seen[v] = 1 when A has had the value v
        int a = 1;                       // the register starts at 1
        seen[a] = 1;

        for (int tick = 1; tick <= 1000; tick++) {
            int is5 = (a == 5);          // decoder output 101
            int step = mux2(2, 4, is5);
            a = addBits(addBits(a, step, 3), c, 3);
            seen[a] = 1;
        }

        printf("  C = %d: A has had the values", c);
        for (int v = 0; v < 8; v++) {
            if (seen[v]) {
                printf(" %d", v);
            }
        }
        printf("   (no official key)\n");
    }
}

// ===================== extra_Q3_15 =====================
// Source: Q3.15.png
// Question: fill in Y1 and Y0 for the four values of the 2-bit input A.
// Circuit in words (my reading of the figure):
//   - a 2:4 decoder with input A
//   - upper OR gate: decoder outputs 00 and 01
//   - lower OR gate: decoder outputs 01, 10 and 11 (three inputs; the 01 wire
//     splits and goes to both OR gates)
//   - two cross-coupled NOR gates:
//       Y1 = NOR(upper OR, Y0)      Y0 = NOR(lower OR, Y1)
// Key: none in the solution folder. Computed here:
//   A1 A0 | Y1 Y0
//    0  0 |  0  1
//    0  1 |  0  0
//    1  0 |  1  0
//    1  1 |  1  0
void extra_Q3_15(void) {
    printf("\nextra_Q3_15: decoder + two OR gates + cross-coupled NOR gates\n");
    printf("  A1 A0 | Y1 Y0\n");
    for (int a = 0; a < 4; a++) {
        int dec[4];
        decoder(a, dec);
        int upper = OR(dec[0], dec[1]);
        int lower = OR(OR(dec[1], dec[2]), dec[3]);

        int try0[2];
        int try1[2];
        norLatch(upper, lower, 0, try0);
        norLatch(upper, lower, 1, try1);

        printf("   %d  %d |", (a >> 1) & 1, a & 1);
        if (try0[0] == try1[0] && try0[1] == try1[1]) {
            printf("  %d  %d\n", try0[0], try0[1]);
        } else {
            printf(" remembers the old value\n");
        }
    }
    printf("  (no official key)\n");
}

// ===================== extra_Q3_18 =====================
// Source: Q3.18.png (the solution is printed in the screenshot)
// Question: the register starts at 0. What are A, B, Y0..Y3 after the first
//   rising clock edge?
// Circuit in words:
//   - a 2-bit adder: constant 3 + B. The adder output is signal A.
//   - a 2-bit register: input A, output B
//   - a 4:1 mux with constant inputs 00->1, 01->0, 10->1, 11->0 and select A
//   - a 2:4 decoder: low input bit (pin 0) = mux output, high bit (pin 1) = constant 1
//   - Y0..Y3 = the decoder outputs 00..11
// Key: A = 10 (binary) = 2, B = 11 (binary) = 3, Y0 = 0, Y1 = 0, Y2 = 0, Y3 = 1
void extra_Q3_18(void) {
    printf("\nextra_Q3_18: adder (+3) + register + mux + decoder, first tick\n");
    int b = 0;                           // the register starts at 0
    int a = addBits(3, b, 2);            // before the tick: A = 3

    b = a;                               // the tick: the register takes A

    a = addBits(3, b, 2);                // 3 + 3 = 6 = 110, two bits kept: 10 = 2
    int m = mux4(1, 0, 1, 0, a);
    int y[4];
    decoder(1 * 2 + m, y);

    printf("  A = %d, B = %d, Y0 = %d, Y1 = %d, Y2 = %d, Y3 = %d\n", a, b, y[0], y[1], y[2], y[3]);
    printf("  key: A = 2, B = 3, Y0 = 0, Y1 = 0, Y2 = 0, Y3 = 1   ");
    okOrWrong(a == 2 && b == 3 && y[0] == 0 && y[1] == 0 && y[2] == 0 && y[3] == 1);
}

// ===================== extra_Q3x8 =====================
// Source: Q3x8.png
// Question: A = 2, C = 3, D = 4 in a stable state. What are A, B, C after the
//   next clock tick?
// Circuit in words (all signals are 4 bits wide):
//   - register 1: input D, output A
//   - an adder: A + C. Its output is B.
//   - register 2: input B, output C
//   - register 3: input C, output D     (D goes back to register 1)
// Key: none in the solution folder. Computed here: A = 4, B = 9, C = 5.
void extra_Q3x8(void) {
    printf("\nextra_Q3x8: three registers and an adder in a ring\n");
    int a = 2, c = 3, d = 4;
    int b = addBits(a, c, 4);            // before the tick: B = 2 + 3 = 5
    printf("  before the tick: A = %d, B = %d, C = %d, D = %d\n", a, b, c, d);

    // The tick: all three registers take their inputs at the same time.
    int nextA = d;
    int nextC = b;
    int nextD = c;
    a = nextA;
    c = nextC;
    d = nextD;
    b = addBits(a, c, 4);                // B settles after the tick

    printf("  after the tick:  A = %d, B = %d, C = %d   (D = %d)   (no official key)\n", a, b, c, d);
}


int main() {
    item3a();
    item4c();
    item6b();
    item7a();
    item9a();
    item11a();
    item12c();
    extra_Q3_14();
    extra_Q3_15();
    extra_Q3_18();
    extra_Q3x8();
}
