#include <stdio.h>

// =====================================================================
// Module 4: Processor Design  -  THE ALU
//
// The ALU (arithmetic logic unit) is the box in the processor that
// calculates. It gets two numbers A and B (N bits each) and a function
// code F (3 bits) that says what to do. The answer comes out on Y.
//
// The ALU in the exam figure (2016-06-09 and 2017-01-12) is built like this:
//
//   F2 (the top bit of F)   0: use B as it is       1: use NOT B, and add 1
//   F1:0 (the two low bits) 0: Y = A AND BB
//                           1: Y = A OR  BB
//                           2: Y = A + BB (+1 if F2 = 1)
//                           3: Y = the sign bit of that sum, zero extended (slt)
//
//   A + NOT B + 1 is the same as A - B (two's complement). So:
//
//   F = 000 (0)  A AND B          F = 100 (4)  A AND NOT B
//   F = 001 (1)  A OR  B          F = 101 (5)  A OR  NOT B
//   F = 010 (2)  A + B            F = 110 (6)  A - B
//   F = 011 (3)  not used         F = 111 (7)  SLT: 1 if A < B, else 0
//
// These items come from the MIPS exams. The ALU itself is the same idea in
// RISC-V, but the RISC-V lecture (lecture 9) numbers the functions another way:
//   000 add, 001 sub, 010 and, 011 or, 101 slt.
// So for a RISC-V exam: read the table that is printed in THAT exam.
//
// Run:  gcc -Wall module4_alu.c -o alu && ./alu
// =====================================================================


// ---------- Helpers ----------

// A mask with the n lowest bits set to 1. Example: n = 8 gives 0xff.
unsigned int maskBits(int n) {
    if (n == 32) {
        return 0xffffffff;          // 1 << 32 is not allowed in C, so do 32 by hand
    }
    return (1u << n) - 1;
}

// The ALU of the exam figure, built from the same parts as the figure.
// a, b = the two inputs, n = number of bits, f = the 3-bit function code.
unsigned int alu(unsigned int a, unsigned int b, int n, int f) {
    unsigned int mask = maskBits(n);
    int f2  = (f >> 2) & 1;          // top bit of F
    int f10 = f & 3;                 // two low bits of F

    // The small mux in the figure: F2 = 0 picks B, F2 = 1 picks NOT B.
    unsigned int bb;
    if (f2 == 1) {
        bb = ~b & mask;
    } else {
        bb = b & mask;
    }

    // The adder. F2 also goes in as "carry in", so F2 = 1 gives A + NOT B + 1 = A - B.
    unsigned int sum = (a + bb + f2) & mask;

    // The big mux on the right: F1:0 picks which result goes out on Y.
    unsigned int y;
    if (f10 == 0) {
        y = a & bb;                  // AND gate
    } else if (f10 == 1) {
        y = a | bb;                  // OR gate
    } else if (f10 == 2) {
        y = sum;                     // the adder
    } else {
        y = (sum >> (n - 1)) & 1;    // bit N-1 (the sign bit) of the sum, zero extended
    }
    return y & mask;
}

// Print our value next to the exam's answer and say if they match.
void check(char* what, unsigned int got, unsigned int key) {
    printf("  %s = 0x%x   key = 0x%x   ", what, got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 1 (a) =====================
// Source: 2016-06-09 Q4.pdf (a)   (the same question is in Q4-Q9.png)
// MIPS-era item. The ALU is the same in RISC-V; only the example instruction
// names change (or / sub / slt exist in RISC-V too).
// Question: N = 8, A = 3, B = 5. What is Y (8-bit hex) for F = 1, F = 6, F = 7,
//           and which instruction uses that function?
// Key: (i) Y = 0x07, an or.  (ii) Y = 0xfe, a sub.  (iii) Y = 0x01, a slt.
void item1a(void) {
    printf("\nItem 1(a): ALU with N = 8, A = 3, B = 5\n");
    unsigned int a = 3;
    unsigned int b = 5;

    // (i) F = 1 = 001: F2 = 0 (B as it is), F1:0 = 1 (OR).   0011 OR 0101 = 0111
    unsigned int y1 = alu(a, b, 8, 1);
    check("(i)   F = 1  Y", y1, 0x07);
    printf("        function: A OR B   -> example instruction: or      key: or\n");

    // (ii) F = 6 = 110: F2 = 1 (NOT B, +1), F1:0 = 2 (adder).   3 - 5 = -2 = 0xfe in 8 bits
    unsigned int y2 = alu(a, b, 8, 6);
    check("(ii)  F = 6  Y", y2, 0xfe);
    printf("        function: A - B    -> example instruction: sub     key: sub\n");

    // (iii) F = 7 = 111: F2 = 1 (subtract), F1:0 = 3 (sign bit of the result).
    // 3 - 5 is negative, so the sign bit is 1. That means "A is less than B".
    unsigned int y3 = alu(a, b, 8, 7);
    check("(iii) F = 7  Y", y3, 0x01);
    printf("        function: SLT      -> example instruction: slt     key: slt\n");
}

// ===================== Item 1 (b) =====================
// Source: 2016-06-09 Q4.pdf (b)   (also in Q4-Q9.png)
// MIPS-era item:  sb $t0,-5($s0)  with $t0 = 0xaa113300, $s0 = 0x11118888.
// In RISC-V the instruction is  sb t0,-5(s0)  and the ALU does the very same
// thing: base register + offset. Only the F code would be 000 with the RISC-V table.
// Question: what are A, F and Y of the ALU?
// Key: A = 0x11118888, F = 0x2, Y = 0x11118883.
//      (The key also accepts the 8-bit answer A = 0x88, F = 0x2, Y = 0x83,
//       because the exam did not say clearly that the ALU is 32 bits.)
void item1b(void) {
    printf("\nItem 1(b): ALU signals while  sb $t0,-5($s0)  executes\n");
    unsigned int s0 = 0x11118888;        // the base register
    int offset = -5;

    // A store must work out the memory ADDRESS = base register + offset.
    // A = the base register value (not the register number!).
    // B = the offset, sign extended to 32 bits (0xfffffffb).
    // $t0 (the value to store) does not go through the ALU at all.
    unsigned int a = s0;
    unsigned int b = (unsigned int)offset;
    int f = 2;                           // 010 = add

    unsigned int y = alu(a, b, 32, f);
    check("A", a, 0x11118888);
    check("F", (unsigned int)f, 0x2);
    check("Y", y, 0x11118883);

    // The 8-bit version that the key also accepts.
    unsigned int y8 = alu(a, b, 8, f);
    printf("  8-bit version (also accepted):\n");
    check("A", a & 0xff, 0x88);
    check("Y", y8, 0x83);
}

// ===================== Item 2 (a), the ALU part =====================
// Source: 2017-01-12 Q4.pdf (a)   (also in Q4-Q4.png)
// MIPS-era item. The whole question (find the instruction) is in
// module4_datapath.c. Here is only the ALU step: signal E = ALUControl = 001.
// Question in short: which function does the ALU do when F = 001?
// Key: OR (that is why the instruction is ori).
void item2a_alu(void) {
    printf("\nItem 2(a), ALU part: which function is F = 001?\n");
    // Try the ALU on two test numbers and see which C operator gives the same answer.
    unsigned int a = 0xc;                // 1100
    unsigned int b = 0xa;                // 1010
    unsigned int y = alu(a, b, 32, 1);
    printf("  alu(1100, 1010, F = 001) = 0x%x\n", y);
    if (y == (a & b)) {
        printf("  same as A AND B   key: OR   WRONG\n");
    } else if (y == (a | b)) {
        printf("  same as A OR B    key: OR   OK\n");
    } else {
        printf("  something else    key: OR   WRONG\n");
    }
}

// ===================== extra_Q4_11 (b), the ALU part =====================
// Source: Q4-Q11.png (b)   (answer is in the capture)
// MIPS-era item:  lw $t2,-4($s0)  with $s0 = 0x00ff000c.
// The figure has the constant 010 wired to the ALU control input.
// The whole question is in module4_datapath.c. Here is only signal S3 = the ALU output.
// Key: S3 = 0x00ff0008.
void extra_Q4_11b_alu(void) {
    printf("\nextra_Q4_11(b), ALU part: ALU output for  lw $t2,-4($s0)\n");
    unsigned int s0 = 0x00ff000c;
    int offset = -4;
    // 010 = add. Address = base + offset.
    unsigned int y = alu(s0, (unsigned int)offset, 32, 2);
    check("S3", y, 0x00ff0008);
}


int main(void) {
    item1a();
    item1b();
    item2a_alu();
    extra_Q4_11b_alu();
    return 0;
}
