#include <stdio.h>

// =====================================================================
// Module 6, Q6: THE OTHER SMALL CALCULATIONS (everything that is not
// Amdahl's law)
//
//   item7c                 how many numbers one AVX instruction handles
//   extra_Q6_TF_Q11_a      one add that adds 4 small numbers at once (demo)
//   extra_Q6_TF_Q11_c      CPI of a loop on a 2-slot multiple-issue processor
//
// Every exam item is its own function. Each one computes its answer,
// prints it and compares it with the exam's answer ("key").
//
// Run:  gcc -Wall module6_misc.c -o misc && ./misc
// =====================================================================


// ---------- Helpers ----------

// Greatest common divisor: the biggest number that divides both a and b.
// Used to shorten a fraction, for example 18/27 -> 2/3 (gcd is 9).
int gcd(int a, int b) {
    while (b != 0) {
        int rest = a % b;
        a = b;
        b = rest;
    }
    return a;
}

// Compare a whole-number answer with the key.
void check_whole(int got, int key) {
    printf("  answer = %d   key = %d   ", got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Compare an exact fraction num/den with the key's fraction keynum/keyden.
// Both are shortened first, so 18/27 and 2/3 count as the same.
void check_fraction(int num, int den, int keynum, int keyden) {
    int g = gcd(num, den);
    int kg = gcd(keynum, keyden);

    printf("  answer = %d/%d = %d/%d (= %g)   key = %d/%d   ",
           num, den, num / g, den / g, (double)num / den, keynum, keyden);
    if (num / g == keynum / kg && den / g == keyden / kg) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 7 (c) =====================
// File: 2018-06-05 Q6.pdf (also in the picture Q6-TF-Q3.png)
// Question: the AVX instruction vaddpd adds double-precision
//           floating-point numbers in parallel. How many at once, if
//           the AVX registers are 256 bits wide?
// Key: 4 double-precision operations in parallel.
void item7c(void) {
    printf("Item 7 (c): operations in parallel for one AVX instruction\n");

    int register_bits = 256;   // width of one AVX register
    int double_bits = 64;      // one double-precision number is 64 bits (8 bytes)

    // The register is cut into equal pieces, one piece for each number.
    // How many pieces fit?
    int in_parallel = register_bits / double_bits;     // 256 / 64 = 4

    check_whole(in_parallel, 4);

    // ---------- ADDED (not in the exam): the same division for other sizes ----------
    // A float (single precision) is 32 bits. Older SSE registers are 128 bits,
    // newer AVX-512 registers are 512 bits.
    int widths[] = {128, 256, 512};
    for (int i = 0; i < 3; i++) {
        printf("  %d-bit register: %d doubles (64 bits each) or %d floats (32 bits each)\n",
               widths[i], widths[i] / 64, widths[i] / 32);
    }
    // --------------------------------------------------------------------------------
}


// ===================== extra_Q6_TF_Q11 (a) =====================
// File: Q6-TF-Q11.png (not from one of the 13 sittings)
// Question: a processor has an add instruction that takes one clock
//           cycle but adds 4 different 16-bit words in parallel. What
//           kind of parallelism is this?
// Key: subword parallelism, or SIMD (single instruction, multiple
//      data), or data-level parallelism. Any one of the three is right.
//
// This is a theory question (see module6_theory.md). Nothing in the key
// is a number, so there is nothing to compare. The code below only
// SHOWS the idea: one "instruction", four additions.
void extra_Q6_TF_Q11_a(void) {
    printf("extra_Q6_TF_Q11 (a): one add instruction, four 16-bit words (demo)\n");

    int word_bits = 16;        // each small number ("subword") is 16 bits
    int words = 4;             // the instruction handles 4 of them

    // The register must hold all four words next to each other.
    int register_bits = words * word_bits;             // 4 * 16 = 64 bits

    // Two "registers", each seen as 4 small numbers.
    int a[] = {1, 2, 3, 4};
    int b[] = {10, 20, 30, 40};
    int sum[4];

    // In hardware these four additions happen AT THE SAME TIME, in one cycle.
    // C has to do them one after the other in a loop, but the idea is the same:
    // the SAME operation (add) on SEVERAL data items.
    for (int i = 0; i < words; i++) {
        sum[i] = a[i] + b[i];
    }

    printf("  %d words * %d bits = one %d-bit register\n", words, word_bits, register_bits);
    printf("  {%d, %d, %d, %d} in one instruction\n", sum[0], sum[1], sum[2], sum[3]);
    printf("  key = subword parallelism / SIMD / data-level parallelism (words, nothing to compare)\n");
}


// ===================== extra_Q6_TF_Q11 (c) =====================
// File: Q6-TF-Q11.png (not from one of the 13 sittings; sub-question (b)
//       is not in the picture)
// Question: this MIPS program
//
//               addi $s0, $0, 8
//               addi $t0, $0, 2
//               addi $t1, $0, 1
//         pow:  mul  $t1, $t1, $t0
//               addi $s0, $s0, -1
//               bne  $s0, $0, pow
//         done:
//
//   runs on a static multiple-issue processor that can do up to
//   2 instructions in each clock cycle. It is scheduled like this:
//
//               Slot 1               Slot 2              Cycle
//               addi $s0, $0, 8      addi $t0, $0, 2       1
//               addi $t1, $0, 1      (empty)               2
//         pow:  mul  $t1, $t1, $t0   addi $s0, $s0, -1     3
//               bne  $s0, $0, pow    (empty)               4
//
//   Compute the CPI (cycles per instruction) as a rational number.
// Key: CPI = 18/27 = 2/3.
//
// The exam is RISC-V only now. The same program in RISC-V:
//               addi s0, zero, 8
//               addi t0, zero, 2
//               addi t1, zero, 1
//         pow:  mul  t1, t1, t0
//               addi s0, s0, -1
//               bne  s0, zero, pow
// The counting is exactly the same.
void extra_Q6_TF_Q11_c(void) {
    printf("extra_Q6_TF_Q11 (c): CPI on a 2-slot multiple-issue processor\n");

    int cycles = 0;            // clock cycles used so far
    int instructions = 0;      // instructions executed so far

    // The three registers of the program.
    int s0;
    int t0;
    int t1;

    // Cycle 1: two instructions at once.
    s0 = 8;                    // addi $s0, $0, 8
    t0 = 2;                    // addi $t0, $0, 2
    cycles = cycles + 1;
    instructions = instructions + 2;

    // Cycle 2: one instruction, the other slot is empty.
    t1 = 1;                    // addi $t1, $0, 1
    cycles = cycles + 1;
    instructions = instructions + 1;

    // The loop "pow". bne jumps back as long as s0 is not 0.
    int rounds = 0;
    do {
        // Cycle 3 of the table: two instructions at once.
        t1 = t1 * t0;          // mul  $t1, $t1, $t0
        s0 = s0 - 1;           // addi $s0, $s0, -1
        cycles = cycles + 1;
        instructions = instructions + 2;

        // Cycle 4 of the table: the branch alone.
        cycles = cycles + 1;   // bne  $s0, $0, pow
        instructions = instructions + 1;

        rounds = rounds + 1;
    } while (s0 != 0);

    // CPI = cycles / instructions.
    // By hand:  cycles       = 2 + 8 * 2 = 18
    //           instructions = 3 + 8 * 3 = 27
    printf("  the loop ran %d times, t1 ended as %d (= 2 to the power 8)\n", rounds, t1);
    printf("  cycles = %d, instructions = %d\n", cycles, instructions);
    check_fraction(cycles, instructions, 18, 27);
}


int main(void) {
    item7c();
    printf("\n");
    extra_Q6_TF_Q11_a();
    printf("\n");
    extra_Q6_TF_Q11_c();
    return 0;
}
