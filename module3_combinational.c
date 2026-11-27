#include <stdio.h>

// =====================================================================
// Module 3 (exam question 3): LOGIC DESIGN, part 1: COMBINATIONAL circuits
//
// A combinational circuit has no memory: the outputs depend only on the
// inputs right now. So we can compute every answer with plain functions.
//
// Every exam item is its own function (item2a, item10a, ...).
// Each one builds the exam circuit from the small gate functions below,
// prints the answer, and compares it with the exam's answer ("key").
//
// Item numbers = exam sittings in date order:
//    1 = 2016-06-09    2 = 2017-01-12    3 = 2017-03-16    4 = 2017-06-08
//    5 = 2018-01-15    6 = 2018-03-15    7 = 2018-06-05    8 = 2019-01-12
//    9 = 2019-03-15   10 = 2019-06-05   11 = 2020-01-13   12 = 2020-03-11
//   13 = 2020-06-05
// "extra_..." items are screenshots that belong to none of these sittings.
// There is no official solution for them, so they print no OK/WRONG.
//
// Run:  gcc -Wall module3_combinational.c -o comb && ./comb
// =====================================================================


// ---------- The building blocks ----------
// Every signal is an int that is 0 or 1.

int AND(int a, int b)  { return a & b; }          // 1 only if both are 1
int OR(int a, int b)   { return a | b; }          // 1 if at least one is 1
int NOT(int a)         { return 1 - a; }          // flips 0 <-> 1
int NAND(int a, int b) { return NOT(AND(a, b)); } // AND, then flipped
int NOR(int a, int b)  { return NOT(OR(a, b)); }  // OR, then flipped
int XOR(int a, int b)  { return a ^ b; }          // 1 if the two are different

// 2:1 multiplexer: the select signal s picks one of the two inputs.
// s = 0 gives d0, s = 1 gives d1.
int mux2(int d0, int d1, int s) {
    if (s == 0) {
        return d0;
    }
    return d1;
}

// 4:1 multiplexer: the 2-bit select signal s (0..3) picks one of four inputs.
// The inputs may be wider than 1 bit (a bus), the mux just passes the value on.
int mux4(int d0, int d1, int d2, int d3, int s) {
    if (s == 0) return d0;
    if (s == 1) return d1;
    if (s == 2) return d2;
    return d3;
}

// 2:4 decoder: a 2-bit input a (0..3) and four outputs.
// Exactly ONE output is 1: the one whose number equals the input ("one-hot").
// y[0] is the output marked 00, y[1] is 01, y[2] is 10, y[3] is 11.
void decoder(int a, int y[4]) {
    for (int i = 0; i < 4; i++) {
        if (i == a) {
            y[i] = 1;
        } else {
            y[i] = 0;
        }
    }
}

// How many bits does a bus need to carry "count" different values?
// 4 values (0..3) need 2 bits, 8 values need 3 bits, ...
int bitsNeeded(int count) {
    int bits = 0;
    while ((1 << bits) < count) {
        bits++;
    }
    return bits;
}


// ---------- Helpers for printing and checking ----------

// Compare one number with the key.
void check(char* what, int got, int key) {
    printf("  %-3s = %d   key = %d   ", what, got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Print the sum-of-products expression for a truth table with inputs A, B, C.
// y[row] is the Y column, row = A*4 + B*2 + C (so row 0 is 000, row 7 is 111).
// Method: every row where Y = 1 gives one product (AND) term. In that term a
// variable that is 0 in the row is written with ' (meaning NOT).
// All the terms are joined with + (OR).
void printSumOfProducts(int y[8], char* n1, char* n2, char* n3) {
    int first = 1;
    printf("  Y = ");
    for (int row = 0; row < 8; row++) {
        if (y[row] == 1) {
            int a = (row >> 2) & 1;      // bit 2 of the row number
            int b = (row >> 1) & 1;      // bit 1
            int c = row & 1;             // bit 0
            if (!first) {
                printf(" + ");
            }
            printf("%s%s%s%s%s%s", n1, a ? "" : "'", n2, b ? "" : "'", n3, c ? "" : "'");
            first = 0;
        }
    }
    printf("      (' means NOT)\n");
}

// Compare two columns of 8 values. Returns 1 if every row is equal.
int sameColumn(int left[8], int right[8]) {
    for (int row = 0; row < 8; row++) {
        if (left[row] != right[row]) {
            return 0;
        }
    }
    return 1;
}

void okOrWrong(int ok) {
    if (ok) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 1 (a) =====================
// Source: 2016-06-09 Q3.pdf (also Q3x6.png)
// Question: construct a combinational circuit with a 2-bit input A and three
//   1-bit outputs B, C, D. Allowed parts: one 4:1 mux, one 2:4 decoder, one
//   2-bit register, one AND, one XOR (not all must be used).
//   Wanted:  A = 0      -> B = 0, C = 0, D = 1
//            A = 1 or 2 -> B = 1, C = 0, D = 0
//            A = 3      -> B = 0, C = 1, D = 0
// Key (a drawing): A goes into the 2:4 decoder.
//   D = decoder output 00.   C = decoder output 11.
//   B = XOR of decoder outputs 01 and 10.
// Here we build the key's circuit and test it against the wanted behaviour.
void item1a(void) {
    printf("\nItem 1(a): decoder + XOR circuit for B, C, D\n");
    int wantB[4] = {0, 1, 1, 0};
    int wantC[4] = {0, 0, 0, 1};
    int wantD[4] = {1, 0, 0, 0};
    int allOk = 1;

    printf("  A | B C D | wanted\n");
    for (int a = 0; a < 4; a++) {
        int y[4];
        decoder(a, y);               // one-hot: only y[a] is 1
        int d = y[0];                // output 00
        int c = y[3];                // output 11
        int b = XOR(y[1], y[2]);     // 1 when A is 1 or 2
        printf("  %d | %d %d %d | %d %d %d\n", a, b, c, d, wantB[a], wantC[a], wantD[a]);
        if (b != wantB[a] || c != wantC[a] || d != wantD[a]) {
            allOk = 0;
        }
    }
    printf("  the key's circuit does what the question asks: ");
    okOrWrong(allOk);
}

// ===================== Item 2 (a) =====================
// Source: 2017-01-12 Q3.pdf (also Q3.12.png)
// Question: make the truth table. Inputs X1, X2, outputs Y1, Y2.
// Circuit in words:
//   - a NOR gate with inputs X1 and X2
//   - a 2:1 mux: input 0 = the NOR output, input 1 = X2, select = X1
//   - a NOT gate with input X2
//   - a 2:4 decoder: A0 (low bit) = mux output, A1 (high bit) = NOT X2
//   - Y1 = decoder output 00,  Y2 = decoder output 11
// Key:  X1 X2 | Y1 Y2
//        0  0 |  0  1
//        0  1 |  1  0
//        1  0 |  0  0
//        1  1 |  0  0
void item2a(void) {
    printf("\nItem 2(a): truth table of NOR + mux + NOT + decoder\n");
    int keyY1[4] = {0, 1, 0, 0};
    int keyY2[4] = {1, 0, 0, 0};

    printf("  X1 X2 | Y1 Y2 | key\n");
    for (int row = 0; row < 4; row++) {
        int x1 = (row >> 1) & 1;
        int x2 = row & 1;

        int norOut = NOR(x1, x2);
        int a0 = mux2(norOut, x2, x1);   // X1 is the select signal
        int a1 = NOT(x2);
        int y[4];
        decoder(a1 * 2 + a0, y);         // A1 is worth 2, A0 is worth 1
        int y1 = y[0];
        int y2 = y[3];

        printf("   %d  %d |  %d  %d | %d %d  ", x1, x2, y1, y2, keyY1[row], keyY2[row]);
        okOrWrong(y1 == keyY1[row] && y2 == keyY2[row]);
    }
}

// ===================== Item 3 (b) =====================
// Source: 2017-03-16 Q3.pdf (also t6.png)
// Question: write a Boolean expression for Y from this truth table.
//   A B C | Y        A B C | Y
//   0 0 0 | 1        1 0 0 | 0
//   0 0 1 | 0        1 0 1 | 0
//   0 1 0 | 0        1 1 0 | 0
//   0 1 1 | 1        1 1 1 | 1
// Key: Y = A'B'C' + A'BC + ABC   (can be simplified to A'B'C' + BC)
void item3b(void) {
    printf("\nItem 3(b): Boolean expression from a truth table\n");
    int y[8] = {1, 0, 0, 1, 0, 0, 0, 1};
    printSumOfProducts(y, "A", "B", "C");

    // Now test the key's two expressions on all 8 rows.
    int keyLong[8];
    int keyShort[8];
    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        int t1 = AND(AND(NOT(a), NOT(b)), NOT(c));   // A'B'C'
        int t2 = AND(AND(NOT(a), b), c);             // A'BC
        int t3 = AND(AND(a, b), c);                  // ABC
        keyLong[row] = OR(OR(t1, t2), t3);
        keyShort[row] = OR(t1, AND(b, c));           // A'B'C' + BC
    }
    printf("  key A'B'C' + A'BC + ABC gives the table: ");
    okOrWrong(sameColumn(keyLong, y));
    printf("  simplified A'B'C' + BC gives the table:  ");
    okOrWrong(sameColumn(keyShort, y));
}

// ===================== Item 5 (c) =====================
// Source: 2018-01-15 Q3.pdf (also Q3.9.png, t5.png)
// Question: (1) write a Boolean expression for this truth table,
//   (2) draw a circuit for it with ONE 2:1 mux and one or two NOT gates.
//   X1 X2 X3 | Y        X1 X2 X3 | Y
//    0  0  0 | 1         1  0  0 | 0
//    0  0  1 | 0         1  0  1 | 0
//    0  1  0 | 1         1  1  0 | 1
//    0  1  1 | 0         1  1  1 | 1
// Key (1): Y = X1'X2'X3' + X1'X2X3' + X1X2X3' + X1X2X3
// Key (2), two possible circuits:
//   circuit 1: select = NOT X1, input 0 = X2,     input 1 = NOT X3
//   circuit 2: select = X1,     input 0 = NOT X3, input 1 = X2
// The idea: when X1 = 0 the table says Y = NOT X3, when X1 = 1 it says Y = X2.
void item5c(void) {
    printf("\nItem 5(c): expression + mux circuit for a truth table\n");
    int y[8] = {1, 0, 1, 0, 0, 0, 1, 1};
    printSumOfProducts(y, "X1", "X2", "X3");

    int circuit1[8];
    int circuit2[8];
    for (int row = 0; row < 8; row++) {
        int x1 = (row >> 2) & 1;
        int x2 = (row >> 1) & 1;
        int x3 = row & 1;
        circuit1[row] = mux2(x2, NOT(x3), NOT(x1));
        circuit2[row] = mux2(NOT(x3), x2, x1);
    }
    printf("  X1 X2 X3 | Y | circuit1 circuit2\n");
    for (int row = 0; row < 8; row++) {
        printf("   %d  %d  %d | %d |    %d        %d\n",
               (row >> 2) & 1, (row >> 1) & 1, row & 1, y[row], circuit1[row], circuit2[row]);
    }
    printf("  key circuit 1 gives the table: ");
    okOrWrong(sameColumn(circuit1, y));
    printf("  key circuit 2 gives the table: ");
    okOrWrong(sameColumn(circuit2, y));
}

// ===================== Item 6 (a) =====================
// Source: 2018-03-15 Q3.pdf (also t4.png)
// Question: prove  A + (B·C) = (A + B)·(A + C)  by perfect induction.
// Perfect induction = try EVERY combination of inputs (here 8 rows) and show
// that the left side and the right side are equal in every row.
// Key: the truth table below; both columns are 0,0,0,1,1,1,1,1.
void item6a(void) {
    printf("\nItem 6(a): prove A + (B·C) = (A + B)·(A + C)\n");
    int keyColumn[8] = {0, 0, 0, 1, 1, 1, 1, 1};
    int left[8];
    int right[8];

    printf("  A B C | A+(B·C) | (A+B)·(A+C)\n");
    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        left[row] = OR(a, AND(b, c));
        right[row] = AND(OR(a, b), OR(a, c));
        printf("  %d %d %d |    %d    |      %d\n", a, b, c, left[row], right[row]);
    }
    printf("  left side = right side in all 8 rows: ");
    okOrWrong(sameColumn(left, right));
    printf("  same column as the key:               ");
    okOrWrong(sameColumn(left, keyColumn));
}

// ===================== Item 9 (b) =====================
// Source: 2019-03-15-riscv Q3.pdf (also t3.png)
// Question: prove  C·NOT(A·B) = C·NOT(A) + C·NOT(B)  by perfect induction.
// Key: the truth table below; both sides are 0,1,0,1,0,1,0,0.
void item9b(void) {
    printf("\nItem 9(b): prove C·(A·B)' = C·A' + C·B'\n");
    int keyColumn[8] = {0, 1, 0, 1, 0, 1, 0, 0};
    int left[8];
    int right[8];

    printf("  A B C | C·(A·B)' | C·A' | C·B' | C·A'+C·B'\n");
    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        left[row] = AND(c, NAND(a, b));          // NAND(a,b) is NOT(A·B)
        int part1 = AND(c, NOT(a));
        int part2 = AND(c, NOT(b));
        right[row] = OR(part1, part2);
        printf("  %d %d %d |    %d     |  %d   |  %d   |     %d\n",
               a, b, c, left[row], part1, part2, right[row]);
    }
    printf("  left side = right side in all 8 rows: ");
    okOrWrong(sameColumn(left, right));
    printf("  same column as the key:               ");
    okOrWrong(sameColumn(left, keyColumn));
}

// ===================== Item 10 (a) =====================
// Source: 2019-06-05-riscv Q3.pdf (also Q3.4.png)
// Question: D0 = 1, D1 = 3, D2 = 2, D3 = 0, E = 3. Give Y0, Y1, Y2 and the
//   width N of the bus.
// Circuit in words:
//   - a 4:1 mux with data inputs D0..D3 (each N bits) and select E (2 bits)
//   - the mux output (N bits) is the input of a 2:4 decoder
//   - Y0 = decoder output 00,  Y1 = decoder output 01
//   - a NAND gate with decoder outputs 10 and 11 as inputs
//   - Y2 = XOR of decoder output 00 and the NAND output
// Key: Y0 = 1, Y1 = 0, Y2 = 0, N = 2
void item10a(void) {
    printf("\nItem 10(a): mux -> decoder -> NAND, XOR\n");
    int m = mux4(1, 3, 2, 0, 3);     // E = 3 picks D3 = 0
    int out[4];
    decoder(m, out);
    int y0 = out[0];
    int y1 = out[1];
    int nandOut = NAND(out[2], out[3]);
    int y2 = XOR(out[0], nandOut);
    // The bus feeds a 2:4 decoder. A decoder with 4 outputs has a 2-bit input.
    int n = bitsNeeded(4);

    printf("  mux output = %d, NAND output = %d\n", m, nandOut);
    check("Y0", y0, 1);
    check("Y1", y1, 0);
    check("Y2", y2, 0);
    check("N", n, 2);
}

// ===================== Item 10 (b) =====================
// Source: 2019-06-05-riscv Q3.pdf (also t2.png)
// Question: write a Boolean expression for Y from this truth table.
//   A B C | Y        A B C | Y
//   0 0 0 | 0        1 0 0 | 0
//   0 0 1 | 1        1 0 1 | 1
//   0 1 0 | 0        1 1 0 | 0
//   0 1 1 | 0        1 1 1 | 1
// Key: Y = A'B'C + AB'C + ABC
void item10b(void) {
    printf("\nItem 10(b): Boolean expression from a truth table\n");
    int y[8] = {0, 1, 0, 0, 0, 1, 0, 1};
    printSumOfProducts(y, "A", "B", "C");

    int key[8];
    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        int t1 = AND(AND(NOT(a), NOT(b)), c);   // A'B'C
        int t2 = AND(AND(a, NOT(b)), c);        // AB'C
        int t3 = AND(AND(a, b), c);             // ABC
        key[row] = OR(OR(t1, t2), t3);
    }
    printf("  key A'B'C + AB'C + ABC gives the table: ");
    okOrWrong(sameColumn(key, y));
}

// ===================== Item 11 (c) =====================
// Source: 2020-01-13-riscv Q3.pdf (also Q3x1.png)
// Question: a 2:4 decoder gets the decimal input 2. What are Y0, Y1, Y2, Y3?
// Key: Y0 = 0, Y1 = 0, Y2 = 1, Y3 = 0
void item11c(void) {
    printf("\nItem 11(c): 2:4 decoder with input 2\n");
    int y[4];
    decoder(2, y);
    check("Y0", y[0], 0);
    check("Y1", y[1], 0);
    check("Y2", y[2], 1);
    check("Y3", y[3], 0);
}

// ===================== Item 12 (a) =====================
// Source: 2020-03-11-riscv Q3.pdf (also t1.png)
// Question: write a Boolean expression for Y from this truth table.
//   A B C | Y        A B C | Y
//   0 0 0 | 1        1 0 0 | 0
//   0 0 1 | 0        1 0 1 | 0
//   0 1 0 | 1        1 1 0 | 1
//   0 1 1 | 1        1 1 1 | 0
// Key: Y = A'B'C' + A'BC' + A'BC + ABC'
void item12a(void) {
    printf("\nItem 12(a): Boolean expression from a truth table\n");
    int y[8] = {1, 0, 1, 1, 0, 0, 1, 0};
    printSumOfProducts(y, "A", "B", "C");

    int key[8];
    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        int t1 = AND(AND(NOT(a), NOT(b)), NOT(c));   // A'B'C'
        int t2 = AND(AND(NOT(a), b), NOT(c));        // A'BC'
        int t3 = AND(AND(NOT(a), b), c);             // A'BC
        int t4 = AND(AND(a, b), NOT(c));             // ABC'
        key[row] = OR(OR(t1, t2), OR(t3, t4));
    }
    printf("  key A'B'C' + A'BC' + A'BC + ABC' gives the table: ");
    okOrWrong(sameColumn(key, y));
}

// ===================== Item 13 (a) =====================
// Source: 2020-06-05-riscv Q3.pdf (also Q3.1.png)
// Question: D0 = 2, D1 = 3, D2 = 0, D3 = 1, E = 3. Give Y0, Y1, Y2 and the
//   width N of the bus.
// Circuit in words:
//   - a 4:1 mux with data inputs D0..D3 (each N bits) and select E (2 bits)
//   - the mux output (N bits) is the input of a 2:4 decoder
//   - Y0 = decoder output 00,  Y1 = decoder output 01
//   - Y2 = XOR of decoder outputs 10 and 11
// Key: Y0 = 0, Y1 = 1, Y2 = 0, N = 2
void item13a(void) {
    printf("\nItem 13(a): mux -> decoder -> XOR\n");
    int m = mux4(2, 3, 0, 1, 3);     // E = 3 picks D3 = 1
    int out[4];
    decoder(m, out);
    int y0 = out[0];
    int y1 = out[1];
    int y2 = XOR(out[2], out[3]);
    int n = bitsNeeded(4);           // input of a 2:4 decoder is 2 bits

    printf("  mux output = %d\n", m);
    check("Y0", y0, 0);
    check("Y1", y1, 1);
    check("Y2", y2, 0);
    check("N", n, 2);
}

// ===================== extra_Q3_17 =====================
// Source: Q3.17.png (same circuit as item 13(a), other numbers)
// Question: D0 = 0, D1 = 2, D2 = 3, D3 = 1, E = 2. Give Y0, Y1, Y2 and N.
// Circuit in words: 4:1 mux (select E) -> 2:4 decoder.
//   Y0 = decoder output 00, Y1 = output 01, Y2 = XOR of outputs 10 and 11.
// Key: none in the solution folder. Computed here: Y0 = 0, Y1 = 0, Y2 = 1, N = 2.
void extra_Q3_17(void) {
    printf("\nextra_Q3_17: mux -> decoder -> XOR\n");
    int m = mux4(0, 2, 3, 1, 2);     // E = 2 picks D2 = 3
    int out[4];
    decoder(m, out);
    int y2 = XOR(out[2], out[3]);
    printf("  mux output = %d\n", m);
    printf("  Y0 = %d, Y1 = %d, Y2 = %d, N = %d   (no official key)\n",
           out[0], out[1], y2, bitsNeeded(4));
}

// ===================== extra_t7 =====================
// Source: t7.png
// Question: which logic gate or building block has this truth table?
//   Y is 1 in every row except A = B = C = 1, where Y = 0.
// Key: none in the solution folder. Computed here: a 3-input NAND gate.
// We try some 3-input gates and print the ones that give the same column.
void extra_t7(void) {
    printf("\nextra_t7: which gate has this truth table?\n");
    int y[8] = {1, 1, 1, 1, 1, 1, 1, 0};
    int and3[8], or3[8], nand3[8], nor3[8], xor3[8];

    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        and3[row]  = AND(AND(a, b), c);
        or3[row]   = OR(OR(a, b), c);
        nand3[row] = NOT(and3[row]);
        nor3[row]  = NOT(or3[row]);
        xor3[row]  = XOR(XOR(a, b), c);
    }
    if (sameColumn(y, and3))  printf("  it is a 3-input AND\n");
    if (sameColumn(y, or3))   printf("  it is a 3-input OR\n");
    if (sameColumn(y, nand3)) printf("  it is a 3-input NAND   (no official key)\n");
    if (sameColumn(y, nor3))  printf("  it is a 3-input NOR\n");
    if (sameColumn(y, xor3))  printf("  it is a 3-input XOR\n");
}

// ===================== extra_t8 =====================
// Source: t8.png
// Question: write a Boolean expression for Y from this truth table.
//   A B C | Y        A B C | Y
//   0 0 0 | 0        1 0 0 | 0
//   0 0 1 | 0        1 0 1 | 0
//   0 1 0 | 1        1 1 0 | 1
//   0 1 1 | 0        1 1 1 | 0
// Key: none in the solution folder. Computed here: Y = A'BC' + ABC'  (= B·C').
void extra_t8(void) {
    printf("\nextra_t8: Boolean expression from a truth table\n");
    int y[8] = {0, 0, 1, 0, 0, 0, 1, 0};
    printSumOfProducts(y, "A", "B", "C");

    // Test the simplified form B·C' on all rows.
    int simple[8];
    for (int row = 0; row < 8; row++) {
        int b = (row >> 1) & 1;
        int c = row & 1;
        simple[row] = AND(b, NOT(c));
    }
    printf("  simplified B·C' gives the table: ");
    okOrWrong(sameColumn(simple, y));
    printf("  (no official key)\n");
}

// ===================== extra_t9 =====================
// Source: t9.png
// Question: prove  (A·B) + (A·C) = A·(B + C)  by perfect induction.
// Key: none in the solution folder. The proof is the table: both sides must be
//   equal in all 8 rows.
void extra_t9(void) {
    printf("\nextra_t9: prove (A·B) + (A·C) = A·(B + C)\n");
    int left[8];
    int right[8];

    printf("  A B C | (A·B)+(A·C) | A·(B+C)\n");
    for (int row = 0; row < 8; row++) {
        int a = (row >> 2) & 1;
        int b = (row >> 1) & 1;
        int c = row & 1;
        left[row] = OR(AND(a, b), AND(a, c));
        right[row] = AND(a, OR(b, c));
        printf("  %d %d %d |      %d      |    %d\n", a, b, c, left[row], right[row]);
    }
    printf("  left side = right side in all 8 rows: ");
    okOrWrong(sameColumn(left, right));
}


int main() {
    item1a();
    item2a();
    item3b();
    item5c();
    item6a();
    item9b();
    item10a();
    item10b();
    item11c();
    item12a();
    item13a();
    extra_Q3_17();
    extra_t7();
    extra_t8();
    extra_t9();
}
