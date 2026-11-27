#include <stdio.h>

// =====================================================================
// Module 3 (exam question 3): LOGIC DESIGN, part 4: ARITHMETIC circuits
//
// - The carry propagate adder (CPA): adds two N-bit numbers and a carry in,
//   gives an N-bit sum and a carry out.
// - Two's complement in N bits: to get -x, invert all bits of x and add 1.
// - Adder/subtractor: A - B is done as A + NOT(B) + 1. The "+ 1" is the
//   carry in of the adder.
//
// Item numbers = exam sittings in date order (see module3_combinational.c).
// "extra_..." items are screenshots that belong to none of the sittings.
//
// Run:  gcc -Wall module3_arithmetic.c -o arith && ./arith
// =====================================================================


// ---------- The building blocks ----------

int AND(int a, int b) { return a & b; }
int OR(int a, int b)  { return a | b; }
int XOR(int a, int b) { return a ^ b; }

// 2:1 mux: s = 0 gives d0, s = 1 gives d1.
int mux2(int d0, int d1, int s) {
    if (s == 0) {
        return d0;
    }
    return d1;
}

// The mask with the n lowest bits set: n = 4 gives 1111 (0xf).
int maskBits(int n) {
    return (1 << n) - 1;
}

// Invert all n bits of a value (an n-bit wide NOT gate).
int invertBits(int value, int n) {
    return (~value) & maskBits(n);
}

// Carry propagate adder with n bits.
// It adds bit by bit from the right, like addition by hand. The carry from
// one bit position goes on ("propagates") to the next one.
// The return value holds the n-bit sum in its n lowest bits and the carry out
// in the bit just above them (bit number n).
int cpa(int a, int b, int carryIn, int n) {
    int sum = 0;
    int carry = carryIn;
    for (int i = 0; i < n; i++) {
        int abit = (a >> i) & 1;         // bit i of a
        int bbit = (b >> i) & 1;         // bit i of b
        // one full adder:
        int s = XOR(XOR(abit, bbit), carry);
        int nextCarry = OR(AND(abit, bbit), AND(carry, XOR(abit, bbit)));
        sum = sum | (s << i);            // put the sum bit in place
        carry = nextCarry;
    }
    return (carry << n) | sum;
}

// Print the lowest n bits of value.
void bits(int value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
}

void okOrWrong(int ok) {
    if (ok) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 4 (b) =====================
// Source: 2017-06-08 Q3.pdf
// Question: what is the 12-bit two's complement representation of -20?
//   Answer in hexadecimal.
// Key: 0xfec
void item4b(void) {
    printf("\nItem 4(b): -20 in 12-bit two's complement\n");
    int n = 12;
    int plus = 20;
    printf("  +20          = ");
    bits(plus, n);
    int inverted = invertBits(plus, n);              // step 1: invert all bits
    printf("\n  inverted     = ");
    bits(inverted, n);
    int minus = (inverted + 1) & maskBits(n);        // step 2: add 1
    printf("\n  add one      = ");
    bits(minus, n);
    printf("\n  answer = 0x%x   key = 0xfec   ", minus);
    okOrWrong(minus == 0xfec);
}

// ===================== Item 5 (b) =====================
// Source: 2018-01-15 Q3.pdf (also Q3x5.png)
// Question: a 4-bit carry propagate adder gets the inputs 0x5 and 0xd, and
//   the carry in is 0x1. What are the output value and the carry out (in hex)?
// Key: output 0x3, carry out 0x1
void item5b(void) {
    printf("\nItem 5(b): 4-bit CPA, 0x5 + 0xd + carry in 1\n");
    int n = 4;
    int result = cpa(0x5, 0xd, 1, n);
    int sum = result & maskBits(n);          // the 4 lowest bits
    int carryOut = (result >> n) & 1;        // the bit above them
    printf("    0101\n  + 1101\n  +    1 (carry in)\n  = %d ", carryOut);
    bits(sum, n);
    printf("   (5 + 13 + 1 = 19 = 16 + 3)\n");
    printf("  output = 0x%x, carry out = 0x%x   key: output 0x3, carry out 0x1   ", sum, carryOut);
    okOrWrong(sum == 0x3 && carryOut == 0x1);
}

// ===================== Item 8 (b) =====================
// Source: 2019-01-12-riscv Q3.pdf (also Q3x4.png)
// Question: construct a circuit with 16-bit inputs A and B (two's complement),
//   a 1-bit input C, and a 16-bit output R.  C = 0: R = A + B.  C = 1: R = A - B.
// Key (a drawing):
//   - a NOT gate (16 bits wide) makes NOT(B)
//   - a 2:1 mux with select C: input 0 = B, input 1 = NOT(B)
//   - a 16-bit carry propagate adder: inputs A and the mux output,
//     carry in = C. The adder output (16 bits) is R.
// Here we build the key's circuit and test it with some numbers.

// The key's circuit. n is the bus width.
int addSub(int a, int b, int c, int n) {
    int notB = invertBits(b, n);
    int second = mux2(b, notB, c);
    int result = cpa(a, second, c, n);
    return result & maskBits(n);         // R = the n sum bits (carry out not used)
}

void item8b(void) {
    printf("\nItem 8(b): 16-bit adder/subtractor circuit\n");
    int n = 16;
    int testA[4] = {100, 7, 1000, -5};
    int testB[4] = {30, 9, 1000, -8};
    int allOk = 1;

    for (int i = 0; i < 4; i++) {
        // Put the test numbers on 16-bit buses (negative ones become two's complement).
        int a = testA[i] & maskBits(n);
        int b = testB[i] & maskBits(n);

        for (int c = 0; c <= 1; c++) {
            int r = addSub(a, b, c, n);
            // What C itself computes, cut to 16 bits:
            int want;
            if (c == 0) {
                want = (testA[i] + testB[i]) & maskBits(n);
            } else {
                want = (testA[i] - testB[i]) & maskBits(n);
            }
            printf("  C = %d: A = %5d, B = %5d  ->  R = 0x%04x   wanted 0x%04x\n",
                   c, testA[i], testB[i], r, want);
            if (r != want) {
                allOk = 0;
            }
        }
    }
    printf("  the key's circuit adds when C = 0 and subtracts when C = 1: ");
    okOrWrong(allOk);
}

// ===================== extra_Q3_16 =====================
// Source: Q3.16.png
// Question: the same adder/subtractor circuit as item 8(b), with N bits and
//   the select called S:  mux input 0 = B, input 1 = NOT(B), select S,
//   adder inputs A and the mux output, carry in = S, output R.
//   (1) N = 4, S = 1, A = 0101, B = 0010. What is R?
//   (2) N = 4, S = 0, A = 0010, R = 1000. What is B?
// Key: none in the solution folder. Computed here: (1) R = 0011, (2) B = 0110.
void extra_Q3_16(void) {
    printf("\nextra_Q3_16: 4-bit adder/subtractor\n");
    int n = 4;

    // (1) S = 1 means subtract: 5 - 2 = 3
    int r = addSub(0x5, 0x2, 1, n);
    printf("  (1) R = ");
    bits(r, n);
    printf("   (no official key)\n");

    // (2) We do not know B, so try all 16 values and keep the one that gives R = 1000.
    for (int b = 0; b < 16; b++) {
        if (addSub(0x2, b, 0, n) == 0x8) {
            printf("  (2) B = ");
            bits(b, n);
            printf("   (no official key)\n");
        }
    }
}

// ===================== extra_Q3_19 =====================
// Source: Q3.19.png (the solution is printed in the screenshot)
// Question: an N-bit adder where BOTH inputs go through a NOT gate first:
//   S = NOT(A) + NOT(B) + Cin.   Cin = 1, N = 4, A = 0xC, B = 0x2. What is S?
// Key: S = 1.  (The key reads it as NOT(A) - B = 3 - 2 = 1, because
//   "+ NOT(B) + 1" is the same as "- B".)
void extra_Q3_19(void) {
    printf("\nextra_Q3_19: adder with both inputs inverted\n");
    int n = 4;
    int notA = invertBits(0xC, n);           // 1100 -> 0011
    int notB = invertBits(0x2, n);           // 0010 -> 1101
    int result = cpa(notA, notB, 1, n);
    int s = result & maskBits(n);
    int carryOut = (result >> n) & 1;

    printf("  NOT(A) = ");
    bits(notA, n);
    printf(", NOT(B) = ");
    bits(notB, n);
    printf(", carry in = 1\n");
    printf("  S = 0x%x (carry out = %d)   key: S = 0x1   ", s, carryOut);
    okOrWrong(s == 0x1);
}


int main() {
    item4b();
    item5b();
    item8b();
    extra_Q3_16();
    extra_Q3_19();
}
