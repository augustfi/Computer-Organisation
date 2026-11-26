#include <stdio.h>

// =====================================================================
// Module 1, second part of Q1: RISC-V ASSEMBLY and MACHINE CODE
//
// Every exam item is its own function (item1b, item2a, ...).
// The program builds each machine code field by field, prints the bits,
// and compares the result with the exam's answer ("key").
//
// Items 1 to 7 come from old MIPS exams. The exam is RISC-V only now, so
// they are done here in their RISC-V version. The MIPS answer is in a comment.
//
// Run:  gcc module1_machine_code.c -o machine && ./machine
// =====================================================================


// ---------- Register numbers (from the RISC-V reference sheet) ----------
// An enum counts from 0, so zero = 0, ra = 1, ... t0 = 5, ... t6 = 31.
enum {
    zero, ra, sp, gp, tp, t0, t1, t2, s0, s1,
    a0, a1, a2, a3, a4, a5, a6, a7,
    s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
    t3, t4, t5, t6
};

// The same names as text, used when printing a decoded instruction.
char* name[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};


// ---------- Helpers ----------

// Print the lowest n bits of value, then one space.
void bits(unsigned int value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        printf("%u", (value >> i) & 1);
    }
    printf(" ");
}

// Compare our machine code with the exam's answer.
void check(unsigned int got, unsigned int key) {
    printf("  machine code = 0x%08x   key = 0x%08x   ", got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Print a 32-bit word split as 7-5-5-3-5-7 bits (the split all formats share).
void split(unsigned int word) {
    printf("  ");
    bits(word >> 25, 7);
    bits(word >> 20, 5);
    bits(word >> 15, 5);
    bits(word >> 12, 3);
    bits(word >> 7, 5);
    bits(word, 7);
    printf("\n");
}


// ---------- The five instruction formats ----------
// Each function puts the fields in their places with << (shift left) and | (or).

// R-type:  funct7 | rs2 | rs1 | funct3 | rd | opcode      (add, xor, sll, sra ...)
unsigned int encodeR(int funct7, int rs2, int rs1, int funct3, int rd, int opcode) {
    unsigned int word = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    printf("  funct7  rs2   rs1   f3  rd    opcode\n");
    split(word);
    return word;
}

// I-type:  imm[11:0] | rs1 | funct3 | rd | opcode          (addi, lw, lb ...)
unsigned int encodeI(int imm, int rs1, int funct3, int rd, int opcode) {
    unsigned int imm12 = imm & 0xfff;          // keep the 12 lowest bits (two's complement if negative)
    unsigned int word = (imm12 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    printf("  imm[11:0]     rs1   f3  rd    opcode\n");
    split(word);
    return word;
}

// S-type:  imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode    (sw, sb ...)
// The 12-bit immediate is cut in two pieces: the top 7 bits and the low 5 bits.
unsigned int encodeS(int imm, int rs2, int rs1, int funct3, int opcode) {
    unsigned int imm12 = imm & 0xfff;
    unsigned int top7 = imm12 >> 5;            // imm[11:5]
    unsigned int low5 = imm12 & 0x1f;          // imm[4:0]
    unsigned int word = (top7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (low5 << 7) | opcode;
    printf("  i[11:5] rs2   rs1   f3  i[4:0] opcode\n");
    split(word);
    return word;
}

// B-type:  imm[12] imm[10:5] | rs2 | rs1 | funct3 | imm[4:1] imm[11] | opcode   (beq, bne ...)
// offset = how many BYTES to jump from the branch itself (negative = backwards).
// Bit 0 of the offset is always 0, so it is not stored.
unsigned int encodeB(int offset, int rs2, int rs1, int funct3, int opcode) {
    unsigned int b12   = (offset >> 12) & 0x1;
    unsigned int b11   = (offset >> 11) & 0x1;
    unsigned int b10_5 = (offset >> 5) & 0x3f;
    unsigned int b4_1  = (offset >> 1) & 0xf;
    unsigned int word = (b12 << 31) | (b10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12)
                      | (b4_1 << 8) | (b11 << 7) | opcode;
    printf("  i[12|10:5] rs2 rs1  f3  i[4:1|11] opcode\n");
    split(word);
    return word;
}

// U-type:  imm[31:12] | rd | opcode                        (lui)
unsigned int encodeU(int imm20, int rd, int opcode) {
    unsigned int word = ((unsigned int)imm20 << 12) | (rd << 7) | opcode;
    printf("  imm[31:12]              rd    opcode\n  ");
    bits(imm20, 20);
    bits(rd, 5);
    bits(opcode, 7);
    printf("\n");
    return word;
}

// A 12-bit immediate is signed. If its top bit (bit 11) is 1 it is negative:
// subtract 2^12 = 4096 to get the real value.
int signExtend12(unsigned int imm12) {
    if (imm12 & 0x800) {
        return (int)imm12 - 4096;
    }
    return (int)imm12;
}


// ===================== Item 1 (b) =====================
// MIPS exam:  sw $t3,-8($a0)  ->  0xAC8BFFF8
// RISC-V:     sw t3,-8(a0)
void item1b(void) {
    printf("\nItem 1(b): machine code of  sw t3,-8(a0)\n");
    // sw is S-type: opcode 0100011 (0x23), funct3 010 (2).
    // rs2 = the register that is STORED (t3), rs1 = the BASE address register (a0).
    unsigned int code = encodeS(-8, t3, a0, 2, 0x23);
    check(code, 0xFFC52C23);
}

// ===================== Item 2 (a) =====================
// MIPS exam:  decode 0x220bfff9  ->  addi $t3,$s0,-7
// RISC-V:     decode 0xFF940E13  ->  addi t3,s0,-7
void item2a(void) {
    printf("\nItem 2(a): which instruction is 0xFF940E13?\n");
    unsigned int word = 0xFF940E13;
    split(word);

    unsigned int opcode = word & 0x7f;           // lowest 7 bits
    unsigned int rd     = (word >> 7) & 0x1f;    // next 5 bits
    unsigned int funct3 = (word >> 12) & 0x7;    // next 3 bits
    unsigned int rs1    = (word >> 15) & 0x1f;   // next 5 bits
    int imm = signExtend12(word >> 20);          // top 12 bits

    printf("  opcode = 0x%02x, funct3 = %u  -> I-type, addi (reference sheet)\n", opcode, funct3);
    printf("  rd = %u (%s), rs1 = %u (%s), imm = %d\n", rd, name[rd], rs1, name[rs1], imm);
    printf("  answer: addi %s,%s,%d     key: addi t3,s0,-7\n", name[rd], name[rs1], imm);
}

// ===================== Item 3 (b) =====================
// t0 contains 0x10008040. Load a word from address 0x10008048 into s1, using t0.
// MIPS exam:  lw $s1,8($t0)  ->  0x8D110008
// RISC-V:     lw s1,8(t0)
void item3b(void) {
    printf("\nItem 3(b): load word from 0x10008048 into s1, t0 = 0x10008040\n");
    int offset = 0x10008048 - 0x10008040;        // how far the address is from t0: 8 bytes
    printf("  offset = %d  ->  lw s1,%d(t0)\n", offset, offset);
    // lw is I-type: opcode 0000011 (0x03), funct3 010 (2).
    unsigned int code = encodeI(offset, t0, 2, s1, 0x03);
    check(code, 0x0082A483);
}

// ===================== Item 4 (a) =====================
// Registers: t2 = 0x0000ff13, t3 = 0x12a30002, s0 = 0x7680bbee.
// Which register changes, and to what?
// MIPS exam:  0x014b8026 = xor $s0,$t2,$t3
// RISC-V:     0x01C3C433 = xor s0,t2,t3
void item4a(void) {
    printf("\nItem 4(a): 0x01C3C433 runs. Which register changes?\n");
    unsigned int reg[32] = {0};
    reg[t2] = 0x0000ff13;
    reg[t3] = 0x12a30002;
    reg[s0] = 0x7680bbee;

    unsigned int word = 0x01C3C433;
    split(word);

    unsigned int opcode = word & 0x7f;
    unsigned int rd     = (word >> 7) & 0x1f;
    unsigned int funct3 = (word >> 12) & 0x7;
    unsigned int rs1    = (word >> 15) & 0x1f;
    unsigned int rs2    = (word >> 20) & 0x1f;
    unsigned int funct7 = word >> 25;

    printf("  opcode = 0x%02x, funct3 = %u, funct7 = %u  -> R-type, xor (reference sheet)\n", opcode, funct3, funct7);
    printf("  instruction: xor %s,%s,%s\n", name[rd], name[rs1], name[rs2]);

    reg[rd] = reg[rs1] ^ reg[rs2];               // ^ is XOR in C
    printf("  answer: %s = 0x%08x     key: s0 = 0x12a3ff11\n", name[rd], reg[rd]);
}

// ===================== Item 5 (b) =====================
// The instruction is stored at address 0x4000aaf0 in LITTLE-ENDIAN format.
// Which byte is at each of the four addresses?
// MIPS exam:  lw $s6,-11($t8) = 0x8F16FFF5  ->  f5, ff, 16, 8f
// RISC-V:     lw s6,-11(t6)   (t6 is used in place of $t8)
void item5b(void) {
    printf("\nItem 5(b): bytes of  lw s6,-11(t6)  in little-endian memory\n");
    unsigned int code = encodeI(-11, t6, 2, s6, 0x03);
    check(code, 0xFF5FAB03);

    // Little-endian: the LOWEST byte of the word is stored at the LOWEST address.
    unsigned int address = 0x4000aaf0;
    for (int i = 0; i < 4; i++) {
        unsigned int byte = (code >> (8 * i)) & 0xff;   // byte number i, counted from the right
        printf("  0x%08x: 0x%02x\n", address + i, byte);
    }
    printf("  key: 03, ab, 5f, ff\n");
}

// ===================== Item 6 (b) =====================
//           addi t1,zero,5
//   loop:   addi t1,t1,-1
//           bne  t1,zero,loop
// MIPS exam:  bne $t1,$zero,loop  ->  0x1520FFFE
// RISC-V:     bne t1,zero,loop
void item6b(void) {
    printf("\nItem 6(b): machine code of  bne t1,zero,loop\n");
    // loop is 1 instruction BEFORE the bne. One instruction = 4 bytes.
    // RISC-V counts the offset from the bne itself, so offset = -4.
    int offset = -1 * 4;
    printf("  offset = %d bytes\n", offset);
    // bne is B-type: opcode 1100011 (0x63), funct3 001 (1).
    unsigned int code = encodeB(offset, zero, t1, 1, 0x63);
    check(code, 0xFE031EE3);
}

// ===================== Item 7 (a) =====================
// MIPS exam:  addi $t3,$t8,-23  ->  0x230BFFE9
// RISC-V:     addi t3,t6,-23    (t6 is used in place of $t8)
void item7a(void) {
    printf("\nItem 7(a): machine code of  addi t3,t6,-23\n");
    // addi is I-type: opcode 0010011 (0x13), funct3 000 (0).
    unsigned int code = encodeI(-23, t6, 0, t3, 0x13);
    check(code, 0xFE9F8E13);
}

// ===================== Item 8 (a) =====================
// Registers: t0 = 0x6, t1 = 0x7, t2 = 0xaf, s0 = 0x23, s1 = 0x3, s2 = 0x1c.
// Which register changes after machine code 0x009293b3, and to what?
void item8a(void) {
    printf("\nItem 8(a): 0x009293b3 runs. Which register changes?\n");
    unsigned int reg[32] = {0};
    reg[t0] = 0x6;
    reg[t1] = 0x7;
    reg[t2] = 0xaf;
    reg[s0] = 0x23;
    reg[s1] = 0x3;
    reg[s2] = 0x1c;

    unsigned int word = 0x009293b3;
    split(word);

    unsigned int opcode = word & 0x7f;
    unsigned int rd     = (word >> 7) & 0x1f;
    unsigned int funct3 = (word >> 12) & 0x7;
    unsigned int rs1    = (word >> 15) & 0x1f;
    unsigned int rs2    = (word >> 20) & 0x1f;
    unsigned int funct7 = word >> 25;

    printf("  opcode = 0x%02x, funct3 = %u, funct7 = %u  -> R-type, sll (reference sheet)\n", opcode, funct3, funct7);
    printf("  instruction: sll %s,%s,%s\n", name[rd], name[rs1], name[rs2]);

    reg[rd] = reg[rs1] << reg[rs2];              // shift left: 6 << 3 = 6 * 8
    printf("  answer: %s = %u     key: t2 = 48\n", name[rd], reg[rd]);
}

// ===================== Item 9 (a) =====================
//   addi t1,zero,-32
//   addi t2,zero,2
//   sra  t0,t1,t2
// i.  Machine code of the sra instruction?
// ii. Value of t0 afterwards (decimal)?
void item9a(void) {
    printf("\nItem 9(a): machine code of  sra t0,t1,t2  and the value of t0\n");
    // sra is R-type: opcode 0110011 (0x33), funct3 101 (5), funct7 0100000 (0x20).
    unsigned int code = encodeR(0x20, t2, t1, 5, t0, 0x33);
    check(code, 0x407352b3);

    int reg[32] = {0};
    reg[t1] = 0 + -32;                           // addi t1,zero,-32
    reg[t2] = 0 + 2;                             // addi t2,zero,2
    reg[t0] = reg[t1] >> reg[t2];                // sra: shift right, copies of the sign bit come in from the left
    printf("  answer: t0 = %d     key: t0 = -8\n", reg[t0]);
}

// ===================== Item 10 (b) =====================
//   addi t0,zero,0x5ff
//   lui  t0,0x32a15
// i.  Machine code of the lui instruction?
// ii. Value of t0 afterwards (hexadecimal)?
void item10b(void) {
    printf("\nItem 10(b): machine code of  lui t0,0x32a15  and the value of t0\n");
    // lui is U-type: opcode 0110111 (0x37).
    unsigned int code = encodeU(0x32a15, t0, 0x37);
    check(code, 0x32a152b7);

    unsigned int reg[32] = {0};
    reg[t0] = 0 + 0x5ff;                         // addi t0,zero,0x5ff
    reg[t0] = 0x32a15 << 12;                     // lui REPLACES the whole register: imm, then 12 zero bits
    printf("  answer: t0 = 0x%08x     key: t0 = 0x32a15000\n", reg[t0]);
}

// ===================== Item 11 (b) =====================
// Machine code of  lb t3,-11(s2)
void item11b(void) {
    printf("\nItem 11(b): machine code of  lb t3,-11(s2)\n");
    // lb is I-type: opcode 0000011 (0x03), funct3 000 (0).
    unsigned int code = encodeI(-11, s2, 0, t3, 0x03);
    check(code, 0xff590e03);
}

// ===================== Item 12 (a) =====================
// Machine code 0xff35ac23 is located at address 0x4310bb10. Which instruction is it?
// (The address is not needed. Only branches and jal use the address.)
void item12a(void) {
    printf("\nItem 12(a): which instruction is 0xff35ac23?\n");
    unsigned int word = 0xff35ac23;
    split(word);

    unsigned int opcode = word & 0x7f;
    unsigned int low5   = (word >> 7) & 0x1f;    // imm[4:0]
    unsigned int funct3 = (word >> 12) & 0x7;
    unsigned int rs1    = (word >> 15) & 0x1f;
    unsigned int rs2    = (word >> 20) & 0x1f;
    unsigned int top7   = word >> 25;            // imm[11:5]
    int imm = signExtend12((top7 << 5) | low5);  // glue the two pieces of the immediate together

    printf("  opcode = 0x%02x, funct3 = %u  -> S-type, sw (reference sheet)\n", opcode, funct3);
    printf("  rs2 = %u (%s), rs1 = %u (%s), imm = %d\n", rs2, name[rs2], rs1, name[rs1], imm);
    printf("  answer: sw %s,%d(%s)     key: sw s3,-8(a1)\n", name[rs2], imm, name[rs1]);
}

// ===================== Item 13 (b) =====================
//            addi t3,zero,220
//   repeat:  addi t3,t3,-2
//            add  s3,s3,s1
//            bne  t3,zero,repeat
// Machine code of the bne instruction?
void item13b(void) {
    printf("\nItem 13(b): machine code of  bne t3,zero,repeat\n");
    // repeat is 2 instructions BEFORE the bne: offset = -2 * 4 = -8 bytes.
    int offset = -2 * 4;
    printf("  offset = %d bytes\n", offset);
    unsigned int code = encodeB(offset, zero, t3, 1, 0x63);
    check(code, 0xfe0e1ce3);
}

int main() {
    item1b();
    item2a();
    item3b();
    item4a();
    item5b();
    item6b();
    item7a();
    item8a();
    item9a();
    item10b();
    item11b();
    item12a();
    item13b();
}
