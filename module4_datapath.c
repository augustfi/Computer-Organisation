#include <stdio.h>

// =====================================================================
// Module 4: Processor Design  -  THE SINGLE-CYCLE DATAPATH
//
// Exam question type: "instruction X is executing. What are the values of
// the wires marked A, B, C, D, E in the figure?"
//
// How this file works:
//   * Every WIRE of the datapath figure is a global variable (w_... and c_...).
//   * mipsWires() / rvWires() take one instruction (32 bits of machine code)
//     and work out the value of every wire, in the same order as the signal
//     travels through the figure: PC -> instruction -> register file ->
//     extend -> ALU -> data memory -> result mux -> next PC.
//   * commit() is the clock edge: it saves the result into the register /
//     memory and moves the PC. We use it to run the instructions that come
//     BEFORE the one in the question.
//   * Each exam item then just reads the wire that each letter sits on.
//     The letters sit on DIFFERENT wires in every exam. The comment at the
//     top of each item says which wire each letter is in that exam's figure.
//
// "unknown": every value has a partner variable ...Known (1 = we know it,
// 0 = we do not). A register nobody told us about is unknown. Anything
// calculated from an unknown value is unknown too. The PC is unknown when
// the exam gives no address. Memory is unknown where nothing was stored.
//
// Items 2-7 and the extra_ items are from the MIPS exams (2016-2018). They
// are kept as MIPS so the answers match the official key. The exam is
// RISC-V now. What differs in RISC-V (same ideas, other details):
//   MIPS                                    RISC-V
//   rs = Instr[25:21], rt = Instr[20:16]    rs1 = Instr[19:15], rs2 = Instr[24:20]
//   rd = Instr[15:11]  (or rt: RegDst mux)  rd  = Instr[11:7]   (always, no RegDst)
//   Sign Extend of Instr[15:0] (16 bits)    Extend of Instr[31:7], ImmSrc picks format
//   branch target = PC + 4 + (imm << 2)     branch target = PC + imm
//   control signal MemToReg                 control signal ResultSrc
//   register numbers: $t0 = 8, $s0 = 16     t0 = 5, s0 = 8   (reference sheet)
// Items 8-13 are RISC-V.
//
// Run:  gcc -Wall module4_datapath.c -o datapath && ./datapath
// =====================================================================


// ---------- RISC-V register numbers (reference sheet) ----------
enum {
    zero, ra, sp, gp, tp, t0, t1, t2, s0, s1,
    a0, a1, a2, a3, a4, a5, a6, a7,
    s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
    t3, t4, t5, t6
};

// ---------- MIPS register numbers (old exams). m_ in front = MIPS. ----------
enum {
    m_zero, m_at, m_v0, m_v1, m_a0, m_a1, m_a2, m_a3,
    m_t0, m_t1, m_t2, m_t3, m_t4, m_t5, m_t6, m_t7,
    m_s0, m_s1, m_s2, m_s3, m_s4, m_s5, m_s6, m_s7,
    m_t8, m_t9, m_k0, m_k1, m_gp, m_sp, m_fp, m_ra
};

char* mipsName[] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};


// ---------- The state of the processor ----------
unsigned int reg[32];        // the 32 registers
int regKnown[32];            // 1 = we know what is in that register

unsigned int pc;             // the program counter
int pcKnown;                 // 0 when the exam gives no address

// Data memory: we only remember the few words that we know.
unsigned int memAddr[16];
unsigned int memData[16];
int memCount;

// ---------- The wires of the figure ----------
unsigned int w_instr;                        // Instr: the 32 bits from the instruction memory
int f_rs, f_rt, f_rd;                        // MIPS fields:   Instr[25:21], [20:16], [15:11]
int f_rs1, f_rs2, f_rdv;                     // RISC-V fields: Instr[19:15], [24:20], [11:7]
unsigned int w_imm;                          // output of Sign Extend (MIPS) / Extend (RISC-V)
unsigned int w_rd1;      int w_rd1Known;     // RD1: first value read from the register file
unsigned int w_rd2;      int w_rd2Known;     // RD2: second value read (also goes to WD of the data memory)
unsigned int w_srcB;     int w_srcBKnown;    // lower ALU input (after the ALUSrc mux)
unsigned int w_alu;      int w_aluKnown;     // ALU result (also the address A of the data memory)
int w_zero;              int w_zeroKnown;    // Zero output of the ALU (1 when the result is 0)
unsigned int w_readData; int w_readKnown;    // RD of the data memory
unsigned int w_result;   int w_resultKnown;  // output of the last mux = WD3 of the register file
int w_writeReg;                              // A3: number of the register that is written
unsigned int w_pcPlus4;                      // PC + 4
unsigned int w_pcTarget;                     // branch target (output of the adder on the right)
unsigned int w_pcNext;   int w_pcNextKnown;  // output of the PC mux = input of the PC register
int w_pcSrc;             int w_pcSrcKnown;   // output of the AND gate (Branch AND Zero)

// ---------- The control signals ----------
int c_RegWrite;      // 1 = write to the register file
int c_RegDst;        // MIPS only: 0 = write register is rt, 1 = rd
int c_ALUSrc;        // 0 = lower ALU input is RD2, 1 = the immediate
int c_Branch;        // 1 = the instruction is a branch
int c_MemWrite;      // 1 = write to the data memory
int c_MemToReg;      // MIPS name. RISC-V calls it ResultSrc. 0 = ALU result, 1 = memory data
int c_Jump;          // MIPS only: 1 for the j instruction
int c_ImmSrc;        // RISC-V only: 0 = I-type, 1 = S-type, 2 = B-type immediate


// ---------- Small helpers ----------

// Forget everything. Every item starts with this.
void resetAll(void) {
    for (int i = 0; i < 32; i++) {
        reg[i] = 0;
        regKnown[i] = 0;
    }
    regKnown[0] = 1;         // register 0 is always 0 (in MIPS and in RISC-V)
    pc = 0;
    pcKnown = 0;
    memCount = 0;
}

// Give a register a value that the exam tells us.
void setReg(int r, unsigned int value) {
    reg[r] = value;
    regKnown[r] = 1;
}

// Remember one word in the data memory.
void memStore(unsigned int address, unsigned int value) {
    for (int i = 0; i < memCount; i++) {
        if (memAddr[i] == address) {
            memData[i] = value;
            return;
        }
    }
    memAddr[memCount] = address;
    memData[memCount] = value;
    memCount++;
}

// Print the lowest n bits of value.
void bits(unsigned int value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        printf("%u", (value >> i) & 1);
    }
}

// Print one value as hex, or the word unknown.
void printValue(unsigned int value, int known) {
    if (known) {
        printf("0x%x", value);
    } else {
        printf("unknown");
    }
}

// Print one signal next to the exam's answer, and OK or WRONG.
// known = 0 means WE say unknown. keyKnown = 0 means THE KEY says unknown.
void show(char* name, unsigned int value, int known, unsigned int key, int keyKnown) {
    printf("  %s = ", name);
    printValue(value, known);
    printf("   key = ");
    printValue(key, keyKnown);
    if (known == keyKnown && (known == 0 || value == key)) {
        printf("   OK\n");
    } else {
        printf("   WRONG\n");
    }
}

// The ALU. op is '+', '-', '&', '|', '^' or '<' (set less than, signed).
unsigned int aluCalc(char op, unsigned int a, unsigned int b) {
    if (op == '-') return a - b;
    if (op == '&') return a & b;
    if (op == '|') return a | b;
    if (op == '^') return a ^ b;
    if (op == '<') return ((int)a < (int)b) ? 1 : 0;
    return a + b;
}

// The parts that are the same in the MIPS figure and the RISC-V figure:
// ALU, data memory, result mux. Called by mipsWires() and rvWires().
// sameReg = 1 when both ALU inputs come from the SAME register.
void aluMemoryResult(char op, int sameReg) {
    // ALUSrc mux: pick the lower ALU input.
    if (c_ALUSrc == 1) {
        w_srcB = w_imm;
        w_srcBKnown = 1;                 // the immediate is part of the instruction: always known
    } else {
        w_srcB = w_rd2;
        w_srcBKnown = w_rd2Known;
    }

    // ALU. The result is known only if both inputs are known.
    w_alu = aluCalc(op, w_rd1, w_srcB);
    w_aluKnown = w_rd1Known && w_srcBKnown;
    // One exception: x - x and x XOR x are 0 whatever x is.
    if (c_ALUSrc == 0 && sameReg && (op == '-' || op == '^')) {
        w_alu = 0;
        w_aluKnown = 1;
    }
    w_zero = (w_alu == 0);
    w_zeroKnown = w_aluKnown;

    // Data memory: the ALU result is the address. RD is the word stored there.
    // (The memory always reads, also when the instruction does not use it.)
    w_readData = 0;
    w_readKnown = 0;
    if (w_aluKnown) {
        for (int i = 0; i < memCount; i++) {
            if (memAddr[i] == w_alu) {
                w_readData = memData[i];
                w_readKnown = 1;
            }
        }
    }

    // Result mux (MemToReg / ResultSrc): 0 = ALU result, 1 = data from memory.
    if (c_MemToReg == 1) {
        w_result = w_readData;
        w_resultKnown = w_readKnown;
    } else {
        w_result = w_alu;
        w_resultKnown = w_aluKnown;
    }

    // AND gate at the top: PCSrc = Branch AND Zero.
    if (c_Branch == 0) {
        w_pcSrc = 0;                     // not a branch: 0, whatever Zero is
        w_pcSrcKnown = 1;
    } else {
        w_pcSrc = w_zero;
        w_pcSrcKnown = w_zeroKnown;
    }
}

// The clock edge: save the result, then move the PC to the next instruction.
void commit(void) {
    if (c_RegWrite == 1 && w_writeReg != 0) {
        reg[w_writeReg] = w_result;
        regKnown[w_writeReg] = w_resultKnown;
    }
    if (c_MemWrite == 1) {
        if (w_aluKnown && w_rd2Known) {
            memStore(w_alu, w_rd2);
        } else {
            memCount = 0;    // stored something we cannot follow: forget the memory, to be safe
        }
    }
    pc = w_pcNext;
    pcKnown = w_pcNextKnown;
}

// Print the main wires, as the step-by-step working of an item.
void printSteps(void) {
    printf("    PC = ");            printValue(pc, pcKnown);
    printf(", PC+4 = ");            printValue(w_pcPlus4, pcKnown);
    printf(", Instr = 0x%08x\n", w_instr);
    printf("    immediate after extend = 0x%x\n", w_imm);
    printf("    RD1 = ");           printValue(w_rd1, w_rd1Known);
    printf(", RD2 = ");             printValue(w_rd2, w_rd2Known);
    printf(", lower ALU input = "); printValue(w_srcB, w_srcBKnown);
    printf("\n    ALU result = ");  printValue(w_alu, w_aluKnown);
    printf(", memory RD = ");       printValue(w_readData, w_readKnown);
    printf(", Result (WD3) = ");    printValue(w_result, w_resultKnown);
    printf("\n    RegWrite=%d ALUSrc=%d Branch=%d MemWrite=%d MemToReg/ResultSrc=%d, write register A3 = %d\n",
           c_RegWrite, c_ALUSrc, c_Branch, c_MemWrite, c_MemToReg, w_writeReg);
    printf("    branch target = ");  printValue(w_pcTarget, pcKnown);
    printf(", next PC = ");          printValue(w_pcNext, w_pcNextKnown);
    printf("\n");
}


// =====================================================================
//                         THE MIPS DATAPATH
// =====================================================================

// ---------- MIPS encoders: put the fields in their places ----------

// R-type: opcode 0 | rs | rt | rd | shamt 0 | funct
unsigned int mipsR(int rs, int rt, int rd, int funct) {
    return (rs << 21) | (rt << 16) | (rd << 11) | funct;
}

// I-type: opcode | rs | rt | 16-bit immediate
unsigned int mipsI(int opcode, int rs, int rt, int imm) {
    return ((unsigned int)opcode << 26) | (rs << 21) | (rt << 16) | (imm & 0xffff);
}

// J-type: opcode 2 | address / 4 (26 bits)
unsigned int mipsJ(unsigned int address) {
    return (2u << 26) | ((address >> 2) & 0x3ffffff);
}

// MIPS opcodes and funct codes used below (MIPS reference sheet):
//   addi 8, lw 0x23, lb 0x20, sw 0x2b, beq 4, j 2
//   R-type (opcode 0): add 0x20, sub 0x22, and 0x24, or 0x25, slt 0x2a

// ---------- One instruction through the MIPS figure ----------
void mipsWires(unsigned int instr) {
    w_instr = instr;

    // 1. Cut the instruction into its fields. The figure does this for EVERY
    //    instruction, also when a field is not used (important for some items).
    int opcode = instr >> 26;
    int funct  = instr & 0x3f;
    f_rs = (instr >> 21) & 0x1f;         // Instr[25:21] -> A1
    f_rt = (instr >> 16) & 0x1f;         // Instr[20:16] -> A2
    f_rd = (instr >> 11) & 0x1f;         // Instr[15:11]

    // 2. Sign Extend: Instr[15:0] becomes 32 bits. If bit 15 is 1, fill with ones.
    w_imm = instr & 0xffff;
    if (w_imm & 0x8000) {
        w_imm = w_imm | 0xffff0000;
    }

    // 3. Control unit: looks at the opcode (and funct) and sets the control signals.
    c_RegWrite = 0; c_RegDst = 0; c_ALUSrc = 0; c_Branch = 0;
    c_MemWrite = 0; c_MemToReg = 0; c_Jump = 0; c_ImmSrc = 0;
    char op = '+';
    if (opcode == 0) {                           // R-type: add, sub, and, or, slt
        c_RegWrite = 1;
        c_RegDst = 1;                            // write register is rd
        if (funct == 0x22) op = '-';
        if (funct == 0x24) op = '&';
        if (funct == 0x25) op = '|';
        if (funct == 0x2a) op = '<';
    } else if (opcode == 0x23 || opcode == 0x20) {   // lw (and lb: this simple figure treats it like lw)
        c_RegWrite = 1;
        c_ALUSrc = 1;                            // ALU adds base + immediate
        c_MemToReg = 1;                          // the register gets the memory data
    } else if (opcode == 0x2b) {                 // sw
        c_ALUSrc = 1;
        c_MemWrite = 1;
    } else if (opcode == 4) {                    // beq
        c_Branch = 1;
        op = '-';                                // subtract: Zero = 1 when the two are equal
    } else if (opcode == 8) {                    // addi
        c_RegWrite = 1;
        c_ALUSrc = 1;
    } else if (opcode == 2) {                    // j
        c_Jump = 1;
    }

    // 4. Register file: read the two registers.
    w_rd1 = reg[f_rs];  w_rd1Known = regKnown[f_rs];
    w_rd2 = reg[f_rt];  w_rd2Known = regKnown[f_rt];

    // 5. RegDst mux: which register number goes to A3.
    if (c_RegDst == 1) {
        w_writeReg = f_rd;
    } else {
        w_writeReg = f_rt;
    }

    // 6. ALU, data memory, result mux, AND gate.
    aluMemoryResult(op, f_rs == f_rt);

    // 7. The PC logic.
    w_pcPlus4 = pc + 4;
    w_pcTarget = w_pcPlus4 + (w_imm << 2);       // MIPS: PC + 4 + (immediate shifted left 2)
    if (c_Jump == 1) {
        w_pcNext = (w_pcPlus4 & 0xf0000000) | ((instr & 0x3ffffff) << 2);
        w_pcNextKnown = pcKnown;
    } else if (w_pcSrc == 1) {
        w_pcNext = w_pcTarget;
        w_pcNextKnown = pcKnown && w_pcSrcKnown;
    } else {
        w_pcNext = w_pcPlus4;
        w_pcNextKnown = pcKnown && w_pcSrcKnown;
    }
}

// sll (shift left by a constant) is not built into this simple figure.
// We only need its effect on the register, so we do it by hand.
void mipsSllByHand(int rd, int rt, int shamt) {
    reg[rd] = reg[rt] << shamt;
    regKnown[rd] = regKnown[rt];
    pc = pc + 4;
}


// ===================== Item 2 (a) =====================
// Source: 2017-01-12 Q4.pdf (a)   (also in Q4-Q4.png)
// MIPS-era item. In RISC-V the same game is played with rs1 = Instr[19:15],
// rs2 = Instr[24:20], rd = Instr[11:7] and a 12-bit immediate.
// Question: the signals are A=10100, B=00100, C=00000, D=1, E=001, F=0,
//           G=0x00000122, H=0xffff2310. Which instruction is executing?
// Wires in this exam's figure:
//   A = Instr[25:21] (rs)       B = Instr[20:16] (rt)      C = Instr[15:11]
//   D = ALUSrc                  E = ALUControl             F = MemWrite
//   G = output of Sign Extend   H = RD2 (value of register rt)
// Key: ori $a0,$s4,0x122
void item2a(void) {
    printf("\nItem 2(a): which instruction gives these signal values?\n");
    int A = 0x14;                    // 10100
    int B = 0x04;                    // 00100
    int C = 0x00;                    // 00000
    int D = 1;
    int E = 1;                       // 001
    int F = 0;
    unsigned int G = 0x00000122;

    // Step 1: find the instruction from the control signals D, E, F.
    // A small table: for each instruction its ALUSrc, ALUControl and MemWrite.
    // ALUControl in the MIPS figure: 010 add, 110 sub, 000 and, 001 or, 111 slt.
    char* names[]    = {"add", "sub", "and", "or", "slt", "addi", "andi", "ori", "slti", "sw", "beq"};
    int tabALUSrc[]  = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0};
    int tabALUCtrl[] = {2, 6, 0, 1, 7, 2, 0, 1, 7, 2, 6};
    int tabMemWr[]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
    int found = -1;
    for (int i = 0; i < 11; i++) {
        if (tabALUSrc[i] == D && tabALUCtrl[i] == E && tabMemWr[i] == F) {
            printf("  ALUSrc = %d, ALUControl = ", D);
            bits(E, 3);
            printf(", MemWrite = %d  fits: %s\n", F, names[i]);
            found = i;
        }
    }
    // D = 1: the ALU uses the immediate. E = 001: the ALU does OR. F = 0: not a store.

    // Step 2: the registers. A is rs (the source), B is rt (the destination of an I-type).
    // Step 3: the immediate is the low 16 bits of G.
    int imm = G & 0xffff;
    printf("  rs = A = %d = %s,  rt = B = %d = %s,  immediate = 0x%x\n",
           A, mipsName[A], B, mipsName[B], imm);

    // Step 4: check with C. Build the machine code and look at Instr[15:11].
    // For an I-type these 5 bits are just the top of the immediate.
    unsigned int code = mipsI(0xd, A, B, imm);       // ori has opcode 0xd
    int c = (code >> 11) & 0x1f;
    printf("  machine code 0x%08x, Instr[15:11] = ", code);
    bits(c, 5);
    printf("  (C in the question = ");
    bits(C, 5);
    printf(")\n");

    // H = 0xffff2310 is RD2 = what is in $a0 right now. It is not needed for the answer.
    printf("  answer: %s %s,%s,0x%x     key: ori $a0,$s4,0x122   ",
           found >= 0 ? names[found] : "?", mipsName[B], mipsName[A], imm);
    if (found == 7 && B == m_a0 && A == m_s4 && imm == 0x122 && c == C) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// ===================== Item 3 (b) =====================
// Source: 2017-03-16 Q4.pdf (b)   (also in Q4-Q5.png)
// MIPS-era item:  lb $t3,-7($s0)  at address 0x004000f0,
//                 $t3 = 0x20040144, $s0 = 0x305502db.
// In RISC-V (lb t3,-7(s0)): the write register is Instr[11:7] = 28 (t3), and
// the branch adder gets the immediate as it is (no << 2), so D would be 0xfffffff9.
// Wires in this exam's figure:
//   A = output of the RegDst mux (number of the register that is written)
//   B = ALU result (the wire into input 0 of the MemToReg mux)
//   C = output of the MemToReg mux (the value written to the register)
//   D = output of the <<2 box (sign-extended immediate, shifted left 2)
// Key: A = 0xb, B = 0x305502d4, C = unknown, D = 0xffffffe4, $t3 after = unknown.
// Why unknown: C is the data read from memory, and nobody says what is stored
// at address 0x305502d4. $t3 gets that value, so it is unknown too.
// (The old value 0x20040144 of $t3 is a trap: it is overwritten.)
void item3b(void) {
    printf("\nItem 3(b): lb $t3,-7($s0)   (MIPS)\n");
    resetAll();
    pc = 0x004000f0;  pcKnown = 1;
    setReg(m_t3, 0x20040144);
    setReg(m_s0, 0x305502db);

    mipsWires(mipsI(0x20, m_s0, m_t3, -7));      // lb: opcode 0x20, rs = base, rt = destination
    printSteps();
    show("A", w_writeReg, 1, 0xb, 1);
    show("B", w_alu, w_aluKnown, 0x305502d4, 1);
    show("C", w_result, w_resultKnown, 0, 0);
    show("D", w_imm << 2, 1, 0xffffffe4, 1);

    commit();                                    // the instruction finishes: $t3 gets the result
    show("$t3 after", reg[m_t3], regKnown[m_t3], 0, 0);
}

// ===================== Item 4 (a) =====================
// Source: 2017-06-08 Q4.pdf (a)   (also in Q4-Q6.png)
// MIPS-era item:  addi $t0,$s0,0x0121  with PC = 0x40001000.
// In RISC-V (addi t0,s0,0x121): A would be 5 (t0), B would be 8 (s0), and the
// branch adder gives PC + imm = 0x40001121 (no +4, no << 2).
// Wires in this exam's figure:
//   A = A3 (number of the register that is written)
//   B = Instr[25:21] (rs, the number that goes into A1)
//   C = output of the AND gate (Branch AND Zero), it steers the PC mux
//   D = output of the MemToReg mux (the value written to the register)
//   E = output of the branch adder = PC + 4 + (immediate << 2)
// Key: A = 8, B = 16, C = 0, D = unknown, E = 0x40001488.
// Why unknown: D = $s0 + 0x121 and the exam does not give $s0.
// E is calculated even though addi is not a branch: the adder always adds.
void item4a(void) {
    printf("\nItem 4(a): addi $t0,$s0,0x0121   (MIPS)\n");
    resetAll();
    pc = 0x40001000;  pcKnown = 1;

    mipsWires(mipsI(8, m_s0, m_t0, 0x0121));
    printSteps();
    show("A", w_writeReg, 1, 8, 1);
    show("B", f_rs, 1, 16, 1);
    show("C", w_pcSrc, w_pcSrcKnown, 0, 1);
    show("D", w_result, w_resultKnown, 0, 0);
    show("E", w_pcTarget, pcKnown, 0x40001488, 1);
}

// ===================== Item 5 (a) =====================
// Source: 2018-01-15 Q4.pdf (a)   (also in Q4-Q1.png)
// MIPS-era item:  lw $t2,-3($s1), nothing else given.
// In RISC-V (lw t2,-3(s1)): write register = 7 (t2), rs1 = 9 (s1),
// and MemToReg is called ResultSrc (also 1 for lw).
// Wires in this exam's figure:
//   A = output of the RegDst mux (number of the register that is written)
//   B = output of Sign Extend
//   C = Instr[25:21] (rs)
//   D = MemToReg (control signal)
//   E = output of the MemToReg mux (the value written to the register)
// Key: A = 10, B = 0xfffffffd, C = 17, D = 1, E = unknown.
// Why unknown: E is the word read from memory; $s1 and the memory are not given.
void item5a(void) {
    printf("\nItem 5(a): lw $t2,-3($s1)   (MIPS)\n");
    resetAll();

    mipsWires(mipsI(0x23, m_s1, m_t2, -3));
    printSteps();
    show("A", w_writeReg, 1, 10, 1);
    show("B", w_imm, 1, 0xfffffffd, 1);
    show("C", f_rs, 1, 17, 1);
    show("D", c_MemToReg, 1, 1, 1);
    show("E", w_result, w_resultKnown, 0, 0);
}

// ===================== Item 6 (b) =====================
// Source: 2018-03-15 Q4.pdf (b)   (also in Q4-Q2.png)
// MIPS-era item. These three lines have run:
//       addi $t0,$zero,0x3f
//       sll  $t1,$t0,4
//       sub  $s0,$t1,$t2
// and now  slt $s1,$s0,$t1  is executing. shamt is encoded as 0.
// In RISC-V (slt s1,s0,t1): the Extend unit would get Instr[31:7], E would be 9 (s1).
// Wires in this exam's figure:
//   A = RD2 (second value read from the register file, the wire down to WD)
//   B = output of the branch adder = PC + 4 + (immediate << 2)
//   C = output of the MemToReg mux (the value written to the register)
//   D = output of Sign Extend
//   E = A3 (number of the register that is written)
// Key: A = 0x3f0, B = unknown, C = unknown, D = 0xffff882a, E = 0x11.
// Why unknown: B needs the PC and no address is given. C is 1 if $s0 < $t1,
// but $s0 = $t1 - $t2 and $t2 was never given, so we cannot tell 0 from 1.
// D: slt has no immediate, but Sign Extend still extends Instr[15:0].
// Those bits are rd (10001), shamt (00000) and funct (101010) = 0x882a.
// Bit 15 is 1, so the upper half is filled with ones.
void item6b(void) {
    printf("\nItem 6(b): slt $s1,$s0,$t1 after addi / sll / sub   (MIPS)\n");
    resetAll();

    mipsWires(mipsI(8, m_zero, m_t0, 0x3f));     // addi $t0,$zero,0x3f
    commit();
    mipsSllByHand(m_t1, m_t0, 4);                // sll  $t1,$t0,4    -> 0x3f0
    mipsWires(mipsR(m_t1, m_t2, m_s0, 0x22));    // sub  $s0,$t1,$t2  -> unknown ($t2 unknown)
    commit();
    printf("  before slt: $t0 = 0x%x, $t1 = 0x%x, $s0 = ", reg[m_t0], reg[m_t1]);
    printValue(reg[m_s0], regKnown[m_s0]);
    printf("\n");

    mipsWires(mipsR(m_s0, m_t1, m_s1, 0x2a));    // slt  $s1,$s0,$t1
    printSteps();
    show("A", w_rd2, w_rd2Known, 0x3f0, 1);
    show("B", w_pcTarget, pcKnown, 0, 0);
    show("C", w_result, w_resultKnown, 0, 0);
    show("D", w_imm, 1, 0xffff882a, 1);
    show("E", w_writeReg, 1, 0x11, 1);
}

// ===================== Item 7 (a) =====================
// Source: 2018-06-05 Q4.pdf (a)   (also in Q4-Q3.png)
// MIPS-era item:  lw $s1,-13($a0)  stored at address 0x40002230.
// In RISC-V (lw s1,-13(a0)): no << 2 box (C would be the plain 0xfffffff3)
// and the write register would be 9 (s1).
// Wires in this exam's figure:
//   A = output of the AND gate (Branch AND Zero), it steers the PC mux
//   B = RD1 (first value read from the register file = value of $a0)
//   C = output of the <<2 box (sign-extended immediate, shifted left 2)
//   D = A3 (number of the register that is written)
//   E = the wire into the PC register (the exam text says so) = next PC
// Key: A = 0x0, B = unknown, C = 0xffffffcc, D = 0x11, E = 0x40002234.
// Why unknown: B is the value of $a0, which is not given.
void item7a(void) {
    printf("\nItem 7(a): lw $s1,-13($a0)   (MIPS)\n");
    resetAll();
    pc = 0x40002230;  pcKnown = 1;

    mipsWires(mipsI(0x23, m_a0, m_s1, -13));
    printSteps();
    show("A", w_pcSrc, w_pcSrcKnown, 0x0, 1);
    show("B", w_rd1, w_rd1Known, 0, 0);
    show("C", w_imm << 2, 1, 0xffffffcc, 1);
    show("D", w_writeReg, 1, 0x11, 1);
    show("E", w_pcNext, w_pcNextKnown, 0x40002234, 1);
}

// ===================== extra_Q4_7 (a) =====================
// Source: Q4-Q7.png (a)   (answer is in the capture; belongs to none of the 13 sittings)
// MIPS-era item:  sw $t2,-8($s0)  with PC = 0x00400020,
//                 $t2 = 0x1200ff00, $s0 = 0x10008012.
// In RISC-V (sw t2,-8(s0)): the stored register is rs2 = Instr[24:20] = 7 (t2),
// and the branch adder gives PC + imm = 0x00400018.
// Wires in this figure:
//   A = Instr[20:16] (rt), the wire into the RegDst mux
//   B = RD2 -> WD of the data memory (the value that is stored)
//   C = ALU result (the wire into input 0 of the MemToReg mux)
//   D = output of the branch adder = PC + 4 + (immediate << 2)
// Key: A = 0xa, B = 0x1200ff00, C = 0x1000800a, D = 0x00400004.
void extra_Q4_7a(void) {
    printf("\nextra_Q4_7(a): sw $t2,-8($s0)   (MIPS)\n");
    resetAll();
    pc = 0x00400020;  pcKnown = 1;
    setReg(m_t2, 0x1200ff00);
    setReg(m_s0, 0x10008012);

    mipsWires(mipsI(0x2b, m_s0, m_t2, -8));      // sw: rs = base, rt = the register that is stored
    printSteps();
    show("A", f_rt, 1, 0xa, 1);
    show("B", w_rd2, w_rd2Known, 0x1200ff00, 1);
    show("C", w_alu, w_aluKnown, 0x1000800a, 1);
    show("D", w_pcTarget, pcKnown, 0x00400004, 1);
}

// ===================== extra_Q4_8 (a) =====================
// Source: Q4-Q8.png (a)   (answer is in the capture; belongs to none of the 13 sittings)
// MIPS-era item. The program:
//             addi $t0,$zero,10
//             addi $t1,$zero,0x3f00
//     loop:   sw   $t0,0($t1)          <- at address 0x04010000
//             addi $t0,$t0,-1
//             addi $t1,$t1,4
//             beq  $zero,$t0,done
//             j    loop
//     done:   addi $v0,$zero,0
// $t1 has the value 0x3f0c and beq is executing. Signals A-E?
// In RISC-V the branch would be  beq zero,t0,done  with rs1 = 0, rs2 = 5,
// and E would be the plain offset 8 (target = PC + 8), no << 2.
// Wires in this figure:
//   A = RD2 (second value read from the register file = value of $t0)
//   B = Instr[25:21] (rs)
//   C = output of the AND gate (Branch AND Zero)
//   D = PC + 4 (the wire into input 0 of the PC mux)
//   E = output of the <<2 box (sign-extended immediate, shifted left 2)
// Key: A = 0x7, B = 0x0, C = 0x0, D = 0x04010010, E = 0x4.
// (The key's text says "bne" in two places. The program has beq. A slip in the text.)
void extra_Q4_8a(void) {
    printf("\nextra_Q4_8(a): beq in the loop, when $t1 = 0x3f0c   (MIPS)\n");
    resetAll();

    // The program as machine code. sw is at 0x04010000 and two instructions
    // come before it, so the first addi is at 0x04010000 - 8.
    unsigned int base = 0x04010000 - 8;
    unsigned int program[8];
    program[0] = mipsI(8, m_zero, m_t0, 10);         // addi $t0,$zero,10
    program[1] = mipsI(8, m_zero, m_t1, 0x3f00);     // addi $t1,$zero,0x3f00
    program[2] = mipsI(0x2b, m_t1, m_t0, 0);         // sw   $t0,0($t1)
    program[3] = mipsI(8, m_t0, m_t0, -1);           // addi $t0,$t0,-1
    program[4] = mipsI(8, m_t1, m_t1, 4);            // addi $t1,$t1,4
    program[5] = mipsI(4, m_zero, m_t0, 1);          // beq  $zero,$t0,done (done is 1 instruction after the next one)
    program[6] = mipsJ(0x04010000);                  // j    loop
    program[7] = mipsI(8, m_zero, m_v0, 0);          // addi $v0,$zero,0
    unsigned int beqAddress = base + 5 * 4;

    // Run the program one instruction at a time until we stand on beq with $t1 = 0x3f0c.
    pc = base;  pcKnown = 1;
    for (int step = 0; step < 100; step++) {
        mipsWires(program[(pc - base) / 4]);
        if (pc == beqAddress && reg[m_t1] == 0x3f0c) {
            break;                                   // stop BEFORE the clock edge: the wires are what we want
        }
        commit();
    }
    printf("  loop has run 3 times: $t0 = 0x%x, $t1 = 0x%x\n", reg[m_t0], reg[m_t1]);
    printSteps();
    show("A", w_rd2, w_rd2Known, 0x7, 1);
    show("B", f_rs, 1, 0x0, 1);
    show("C", w_pcSrc, w_pcSrcKnown, 0x0, 1);
    show("D", w_pcPlus4, pcKnown, 0x04010010, 1);
    show("E", w_imm << 2, 1, 0x4, 1);
}

// ===================== extra_Q4_10 (a) =====================
// Source: Q4-Q10.png (a)   (answer is in the capture; the header says "Module 3",
//         the numbering of an older year)
// MIPS-era item: the register file only. Inst = 0x01484824.
//   $t0 = 0x00000007, $t1 = 0x00000039, $t2 = 0x000000f0.
// In RISC-V the fields would be cut at [19:15], [24:20], [11:7] instead.
// Wires in this figure:
//   S1 = WE3 (write enable of the register file = RegWrite)
//   S2 = RD1 (first value read)
//   S3 = WD3 (the value written to the register)
// Key: S1 = 0x1, S2 = 0x000000f0, S3 = 0x00000000.
// The instruction decodes to  and $t1,$t2,$t0:  0xf0 AND 0x07 = 0.
void extra_Q4_10a(void) {
    printf("\nextra_Q4_10(a): register file signals for Inst = 0x01484824   (MIPS)\n");
    resetAll();
    setReg(m_t0, 0x00000007);
    setReg(m_t1, 0x00000039);
    setReg(m_t2, 0x000000f0);

    unsigned int inst = 0x01484824;
    mipsWires(inst);
    printf("  opcode = %u, rs = %d (%s), rt = %d (%s), rd = %d (%s), funct = 0x%x (and)\n",
           inst >> 26, f_rs, mipsName[f_rs], f_rt, mipsName[f_rt], f_rd, mipsName[f_rd], inst & 0x3f);
    printf("  instruction: and %s,%s,%s\n", mipsName[f_rd], mipsName[f_rs], mipsName[f_rt]);
    show("S1", c_RegWrite, 1, 0x1, 1);
    show("S2", w_rd1, w_rd1Known, 0x000000f0, 1);
    show("S3", w_result, w_resultKnown, 0x00000000, 1);
}

// ===================== extra_Q4_11 (b) =====================
// Source: Q4-Q11.png (b)   (answer is in the capture; header says "Module 3")
// MIPS-era item: a small datapath that can only do lw and sw.
//   memory: address 0x00ff0008 holds 0x00000014, address 0x00ff0004 holds 0x00000010
//   $t2 = 0x00000020, $s0 = 0x00ff000c.   Instruction:  lw $t2,-4($s0)
// In RISC-V (lw t2,-4(s0)) the values are the same; only the field positions differ.
// Wires in this figure:
//   S1 = WE3 (RegWrite)          S2 = WE of the data memory (MemWrite)
//   S3 = ALU result (the memory address)
//   S4 = WD3 (the value written to the register = the word read from memory)
//   S5 = RD2 -> WD of the data memory (value of register rt = $t2, the OLD value)
// Key: S1 = 1, S2 = 0, S3 = 0x00ff0008, S4 = 0x00000014, S5 = 0x00000020 (or "don't care").
// S5 is "don't care" because lw does not write to memory, but the wire still has a value.
void extra_Q4_11b(void) {
    printf("\nextra_Q4_11(b): lw $t2,-4($s0) on the lw/sw datapath   (MIPS)\n");
    resetAll();
    memStore(0x00ff0008, 0x00000014);
    memStore(0x00ff0004, 0x00000010);
    setReg(m_t2, 0x00000020);
    setReg(m_s0, 0x00ff000c);

    mipsWires(mipsI(0x23, m_s0, m_t2, -4));
    printSteps();
    show("S1", c_RegWrite, 1, 1, 1);
    show("S2", c_MemWrite, 1, 0, 1);
    show("S3", w_alu, w_aluKnown, 0x00ff0008, 1);
    show("S4", w_result, w_resultKnown, 0x00000014, 1);
    show("S5", w_rd2, w_rd2Known, 0x00000020, 1);
}

// ===================== extra_Q4_12 (a) =====================
// Source: Q4-Q12.png (a)   (English and Swedish text; answer is in the capture)
// MIPS-era item:  slt $t0,$t1,$t3.  Values of MemToReg, ALUSrc, Branch and A3?
// In RISC-V (slt t0,t1,t3): ResultSrc = 0, ALUSrc = 0, Branch = 0, A3 = 00101 (t0 = 5).
// Key: MemToReg = 0, ALUSrc = 0, Branch = 0, A3 = 01000 (binary).
//   MemToReg = 0: the register gets the ALU result, not memory data.
//   ALUSrc = 0:   the ALU compares two registers, no immediate.
//   Branch = 0:   not a branch.
//   A3 = rd = $t0 = register number 8.
void extra_Q4_12a(void) {
    printf("\nextra_Q4_12(a): control signals for slt $t0,$t1,$t3   (MIPS)\n");
    resetAll();

    mipsWires(mipsR(m_t1, m_t3, m_t0, 0x2a));    // slt rd = $t0, rs = $t1, rt = $t3
    show("MemToReg", c_MemToReg, 1, 0, 1);
    show("ALUSrc", c_ALUSrc, 1, 0, 1);
    show("Branch", c_Branch, 1, 0, 1);
    printf("  A3 in binary = ");
    bits(w_writeReg, 5);
    printf("   key = 01000\n");
    show("A3", w_writeReg, 1, 0x8, 1);
}


// =====================================================================
//                        THE RISC-V DATAPATH
// =====================================================================

// ---------- RISC-V encoders (same layouts as in module1_machine_code.c) ----------

// R-type: funct7 | rs2 | rs1 | funct3 | rd | opcode 0x33
unsigned int rvR(int funct7, int rs2, int rs1, int funct3, int rd) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x33;
}

// I-type: imm[11:0] | rs1 | funct3 | rd | opcode   (addi/ori: 0x13, lw: 0x03)
unsigned int rvI(int imm, int rs1, int funct3, int rd, int opcode) {
    unsigned int imm12 = imm & 0xfff;
    return (imm12 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

// S-type (sw): imm[11:5] | rs2 | rs1 | funct3 010 | imm[4:0] | opcode 0x23
unsigned int rvS(int imm, int rs2, int rs1) {
    unsigned int imm12 = imm & 0xfff;
    return ((imm12 >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm12 & 0x1f) << 7) | 0x23;
}

// B-type (beq): imm[12] imm[10:5] | rs2 | rs1 | funct3 000 | imm[4:1] imm[11] | opcode 0x63
// offset = how many BYTES from the branch itself to the target.
unsigned int rvB(int offset, int rs2, int rs1) {
    unsigned int b12   = (offset >> 12) & 0x1;
    unsigned int b11   = (offset >> 11) & 0x1;
    unsigned int b10_5 = (offset >> 5) & 0x3f;
    unsigned int b4_1  = (offset >> 1) & 0xf;
    return (b12 << 31) | (b10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (b4_1 << 8) | (b11 << 7) | 0x63;
}

// ---------- One instruction through the RISC-V figure ----------
void rvWires(unsigned int instr) {
    w_instr = instr;

    // 1. Cut the instruction into its fields. Always the same places in RISC-V.
    int opcode = instr & 0x7f;
    int funct3 = (instr >> 12) & 0x7;
    int funct7 = instr >> 25;
    f_rs1 = (instr >> 15) & 0x1f;        // Instr[19:15] -> A1
    f_rs2 = (instr >> 20) & 0x1f;        // Instr[24:20] -> A2
    f_rdv = (instr >> 7) & 0x1f;         // Instr[11:7]  -> A3
    w_writeReg = f_rdv;

    // 2. Control unit (table in lecture 9).
    c_RegWrite = 0; c_RegDst = 0; c_ALUSrc = 0; c_Branch = 0;
    c_MemWrite = 0; c_MemToReg = 0; c_Jump = 0; c_ImmSrc = 0;
    char op = '+';
    if (opcode == 0x33) {                        // R-type: add, sub, xor, or, and, slt
        c_RegWrite = 1;
        if (funct3 == 0 && funct7 == 0x20) op = '-';
        if (funct3 == 2) op = '<';
        if (funct3 == 4) op = '^';
        if (funct3 == 6) op = '|';
        if (funct3 == 7) op = '&';
    } else if (opcode == 0x13) {                 // I-type calculation: addi, xori, ori, andi
        c_RegWrite = 1;
        c_ALUSrc = 1;
        c_ImmSrc = 0;
        if (funct3 == 4) op = '^';
        if (funct3 == 6) op = '|';
        if (funct3 == 7) op = '&';
    } else if (opcode == 0x03) {                 // lw
        c_RegWrite = 1;
        c_ALUSrc = 1;
        c_ImmSrc = 0;
        c_MemToReg = 1;                          // ResultSrc = 1: the register gets the memory data
    } else if (opcode == 0x23) {                 // sw
        c_ALUSrc = 1;
        c_ImmSrc = 1;
        c_MemWrite = 1;
    } else if (opcode == 0x63) {                 // beq
        c_ImmSrc = 2;
        c_Branch = 1;
        op = '-';                                // subtract: Zero = 1 when the two are equal
    }

    // 3. Extend unit: ImmSrc says where the immediate bits sit in Instr[31:7].
    if (c_ImmSrc == 0) {                         // I-type: Instr[31:20]
        w_imm = instr >> 20;
        if (w_imm & 0x800) w_imm = w_imm | 0xfffff000;
    } else if (c_ImmSrc == 1) {                  // S-type: Instr[31:25] and Instr[11:7]
        w_imm = ((instr >> 25) << 5) | ((instr >> 7) & 0x1f);
        if (w_imm & 0x800) w_imm = w_imm | 0xfffff000;
    } else {                                     // B-type: bits 12, 10:5, 4:1, 11, and bit 0 is 0
        w_imm = ((instr >> 31) << 12) | (((instr >> 7) & 1) << 11)
              | (((instr >> 25) & 0x3f) << 5) | (((instr >> 8) & 0xf) << 1);
        if (w_imm & 0x1000) w_imm = w_imm | 0xffffe000;
    }

    // 4. Register file: read the two registers.
    w_rd1 = reg[f_rs1];  w_rd1Known = regKnown[f_rs1];
    w_rd2 = reg[f_rs2];  w_rd2Known = regKnown[f_rs2];

    // 5. ALU, data memory, result mux, AND gate.
    aluMemoryResult(op, f_rs1 == f_rs2);

    // 6. The PC logic.
    w_pcPlus4 = pc + 4;
    w_pcTarget = pc + w_imm;                     // RISC-V: PC + immediate (no +4, no << 2)
    if (w_pcSrc == 1) {
        w_pcNext = w_pcTarget;
    } else {
        w_pcNext = w_pcPlus4;
    }
    w_pcNextKnown = pcKnown && w_pcSrcKnown;
}

// lui and "shift left by a constant" are not built into this simple figure.
// We only need what they do to the register, so we do them by hand.
void rvLuiByHand(int rd, unsigned int imm20) {
    reg[rd] = imm20 << 12;               // lui puts the 20 bits at the top, 12 zeros below
    regKnown[rd] = 1;
    pc = pc + 4;
}

void rvShiftLeftByHand(int rd, int rs1, int shamt) {
    reg[rd] = reg[rs1] << shamt;
    regKnown[rd] = regKnown[rs1];
    pc = pc + 4;
}


// ===================== Item 8 (a) =====================
// Source: 2019-01-12-riscv Q4.pdf (a)   (also in Q4design6.png)
// RISC-V. These four lines have run, the first addi is at 0x00040210:
//       addi t0,zero,0x3f0
//       addi t0,t0,16
//       sw   t0,-8(t0)
//       addi t0,t0,12
// and now  lw s2,-20(t0)  is executing.
// Wires in this exam's figure:
//   A = output of Extend
//   B = ALU result (the wire that goes over the data memory into input 0 of the ResultSrc mux)
//   C = Instr[11:7] (rd, the number that goes into A3)
//   D = PC + 4 (the wire into input 0 of the PC mux)
//   E = output of the ResultSrc mux (the value written to the register)
// Key: A = 0xffffffec, B = 0x3f8, C = 0x12, D = 0x40224, E = 0x4000.
//
// *** E DOES NOT MATCH THE KEY, AND THE KEY LOOKS WRONG. ***
// Step by step: t0 = 0x3f0, then 0x3f0 + 16 = 0x400. sw stores t0 (0x400) at
// address 0x400 - 8 = 0x3f8. Then t0 = 0x40c. lw reads address 0x40c - 20 = 0x3f8
// (this is the key's own B), so it reads back 0x400. E = 0x400, not 0x4000.
// This exam was rewritten from MIPS to RISC-V in 2025. The key's 0x4000 fits
// larger MIPS constants (for example 0x3ff0 + 16 = 0x4000) that do not fit in
// a 12-bit RISC-V immediate. It looks like the numbers in the question were
// made smaller and E in the key was not updated. This file does not force a match.
void item8a(void) {
    printf("\nItem 8(a): lw s2,-20(t0) after addi / addi / sw / addi   (RISC-V)\n");
    resetAll();
    pc = 0x00040210;  pcKnown = 1;

    rvWires(rvI(0x3f0, zero, 0, t0, 0x13));      // addi t0,zero,0x3f0
    commit();
    rvWires(rvI(16, t0, 0, t0, 0x13));           // addi t0,t0,16      -> 0x400
    commit();
    rvWires(rvS(-8, t0, t0));                    // sw   t0,-8(t0)     -> memory[0x3f8] = 0x400
    commit();
    rvWires(rvI(12, t0, 0, t0, 0x13));           // addi t0,t0,12      -> 0x40c
    commit();
    printf("  before lw: t0 = 0x%x, memory[0x%x] = 0x%x\n", reg[t0], memAddr[0], memData[0]);

    rvWires(rvI(-20, t0, 2, s2, 0x03));          // lw   s2,-20(t0)
    printSteps();
    show("A", w_imm, 1, 0xffffffec, 1);
    show("B", w_alu, w_aluKnown, 0x3f8, 1);
    show("C", f_rdv, 1, 0x12, 1);
    show("D", w_pcPlus4, pcKnown, 0x40224, 1);
    // E: printed by hand, because here we do not agree with the key (see the comment above).
    printf("  E = ");
    printValue(w_result, w_resultKnown);
    printf("   key = 0x4000   ");
    if (w_resultKnown && w_result == 0x4000) {
        printf("OK\n");
    } else {
        printf("DIFFERENT - the key looks wrong here (0x400 is what the code gives), read the comment\n");
    }
}

// ===================== Item 9 (a) =====================
// Source: 2019-03-15-riscv Q4.pdf (a)   (also in Q4design5.png)
// RISC-V:  sw t2,-8(a0)  with t2 = 0x2f, a0 = 0x3500.
// NOTE: the question text says "single-cycle 32-bit MIPS processor". That is a
// slip of the 2025 rewrite: the figure and the registers are RISC-V.
// Wires in this exam's figure:
//   A = Instr[19:15] (rs1, the number that goes into A1)
//   B = RD2 -> WD of the data memory (the value that is stored)
//   C = ALU result (the wire into input 0 of the ResultSrc mux)
//   D = PC (the wire from the PC register to the branch adder)
//   E = RegWrite (control signal into WE3)
// Key: A = 0xA, B = 0x2f, C = 0x34f8, D = unknown, E = 0x0.
// Why unknown: D is the PC and the exam gives no address for the instruction.
void item9a(void) {
    printf("\nItem 9(a): sw t2,-8(a0)   (RISC-V)\n");
    resetAll();
    setReg(t2, 0x2f);
    setReg(a0, 0x3500);

    rvWires(rvS(-8, t2, a0));                    // sw: rs2 = t2 (stored), rs1 = a0 (base)
    printSteps();
    show("A", f_rs1, 1, 0xA, 1);
    show("B", w_rd2, w_rd2Known, 0x2f, 1);
    show("C", w_alu, w_aluKnown, 0x34f8, 1);
    show("D", pc, pcKnown, 0, 0);
    show("E", c_RegWrite, 1, 0x0, 1);
}

// ===================== Item 10 (a) =====================
// Source: 2019-06-05-riscv Q4.pdf (a)   (also in Q4design4.png)
// RISC-V. These two lines have run:
//       addi t0,zero,3
//       sll  t1,t0,3        (the exam writes sll with a constant; it means slli)
// and now  sub s0,t1,t0  is executing.
// Wires in this exam's figure:
//   A = Instr[24:20] (rs2, the number that goes into A2)
//   B = RD1 (first value read from the register file = value of t1)
//   C = ALUSrc (control signal)
//   D = PC (output of the PC register)
//   E = output of the ResultSrc mux (the value written to the register)
// Key: A = 0x5, B = 0x18, C = 0x0, D = unknown, E = 0x15.
// Why unknown: D is the PC and no address is given.
void item10a(void) {
    printf("\nItem 10(a): sub s0,t1,t0 after addi / sll   (RISC-V)\n");
    resetAll();

    rvWires(rvI(3, zero, 0, t0, 0x13));          // addi t0,zero,3
    commit();
    rvShiftLeftByHand(t1, t0, 3);                // sll  t1,t0,3   -> 3 * 8 = 24 = 0x18
    printf("  before sub: t0 = 0x%x, t1 = 0x%x\n", reg[t0], reg[t1]);

    rvWires(rvR(0x20, t0, t1, 0, s0));           // sub  s0,t1,t0  (rs1 = t1, rs2 = t0)
    printSteps();
    show("A", f_rs2, 1, 0x5, 1);
    show("B", w_rd1, w_rd1Known, 0x18, 1);
    show("C", c_ALUSrc, 1, 0x0, 1);
    show("D", pc, pcKnown, 0, 0);
    show("E", w_result, w_resultKnown, 0x15, 1);
}

// ===================== Item 11 (a) =====================
// Source: 2020-01-13-riscv Q4.pdf (a)   (also in Q4design3.png)
// RISC-V. The program:
//             xor  t1,t2,t2
//             beq  t1,zero,next       <- at address 0x40004008
//             addi t0,t0,1
//     next:
// NOTE: the question says "Suppose the bne instruction is executing" in one
// place and beq everywhere else. A slip of the 2025 rewrite. The key fits beq
// (with bne the branch would NOT be taken and C would be 0), so this is done as beq.
// Wires in this exam's figure:
//   A = Instr[19:15] (rs1, the number that goes into A1)
//   B = ImmSrc (control signal into Extend: 0 = I-type, 1 = S-type, 2 = B-type)
//   C = output of the AND gate (Branch AND Zero), it steers the PC mux
//   D = branch target (the wire into input 1 of the PC mux) = PC + immediate
//   E = RD2 -> WD of the data memory (value of register rs2 = zero)
// Key: A = 0x6, B = 0x2, C = 0x1, D = 0x40004010, E = 0x0.
// Trick: t2 is unknown, but t2 XOR t2 is 0 for every value. So t1 = 0,
// the branch compares 0 with 0, and it IS taken (C = 1).
void item11a(void) {
    printf("\nItem 11(a): beq t1,zero,next after xor t1,t2,t2   (RISC-V)\n");
    resetAll();
    pc = 0x40004008 - 4;  pcKnown = 1;           // xor is the instruction just before beq

    rvWires(rvR(0, t2, t2, 4, t1));              // xor t1,t2,t2  -> 0
    commit();
    printf("  before beq: t1 = ");
    printValue(reg[t1], regKnown[t1]);
    printf("\n");

    // next is 2 instructions after beq = 8 bytes.
    rvWires(rvB(8, zero, t1));                   // beq t1,zero,next  (rs1 = t1, rs2 = zero)
    printSteps();
    show("A", f_rs1, 1, 0x6, 1);
    show("B", c_ImmSrc, 1, 0x2, 1);
    show("C", w_pcSrc, w_pcSrcKnown, 0x1, 1);
    show("D", w_pcTarget, pcKnown, 0x40004010, 1);
    show("E", w_rd2, w_rd2Known, 0x0, 1);
}

// ===================== Item 12 (b) =====================
// Source: 2020-03-11-riscv Q4.pdf (b)   (also in Q4design2.png)
// RISC-V. The program, lui is at 0x40004000:
//       lui  t1,0x22
//       ori  t0,t1,0x11
//       sw   t0,8(s1)
//       addi t1,s1,0
//       lw   s2,0(t1)
//       add  t4,s2,t0          <- executing
// Wires in this exam's figure:
//   A = Instr[19:15] (rs1, the number that goes into A1)
//   B = RD1 (first value read from the register file = value of s2)
//   C = RegWrite (control signal into WE3)
//   D = RD2 -> WD of the data memory (value of t0)
//   E = PC + 4 (the wire into input 0 of the PC mux)
// Key: A = 0x12, B = unknown, C = 0x1, D = 0x00022011, E = 0x40004018.
// Why unknown: s1 is never given. sw stores t0 at address s1 + 8, but lw loads
// from address s1 + 0. That is a different word, and nobody says what is in it.
// (Trap: it looks as if lw reads back what sw stored. It does not.)
void item12b(void) {
    printf("\nItem 12(b): add t4,s2,t0 at the end of a six-line program   (RISC-V)\n");
    resetAll();
    pc = 0x40004000;  pcKnown = 1;

    rvLuiByHand(t1, 0x22);                       // lui  t1,0x22      -> 0x00022000
    rvWires(rvI(0x11, t1, 6, t0, 0x13));         // ori  t0,t1,0x11   -> 0x00022011
    commit();
    rvWires(rvS(8, t0, s1));                     // sw   t0,8(s1)     (address unknown: s1 unknown)
    commit();
    rvWires(rvI(0, s1, 0, t1, 0x13));            // addi t1,s1,0      -> unknown
    commit();
    rvWires(rvI(0, t1, 2, s2, 0x03));            // lw   s2,0(t1)     -> unknown
    commit();
    printf("  before add: t0 = 0x%x, s2 = ", reg[t0]);
    printValue(reg[s2], regKnown[s2]);
    printf("\n");

    rvWires(rvR(0, t0, s2, 0, t4));              // add  t4,s2,t0  (rs1 = s2, rs2 = t0)
    printSteps();
    show("A", f_rs1, 1, 0x12, 1);
    show("B", w_rd1, w_rd1Known, 0, 0);
    show("C", c_RegWrite, 1, 0x1, 1);
    show("D", w_rd2, w_rd2Known, 0x00022011, 1);
    show("E", w_pcPlus4, pcKnown, 0x40004018, 1);
}

// ===================== Item 13 (a) =====================
// Source: 2020-06-05-riscv Q4.pdf (a)   (also in Q4design1.png)
// RISC-V. The program, lui is at 0x40004044:
//       lui  t2,0x003f0
//       ori  t2,t2,0x011
//       lw   t8,4(t2)          <- executing
// NOTE: RISC-V has no register t8 (it has t0-t6). A leftover from the MIPS
// original. None of A-E depends on the destination register, so this file
// uses t3 in its place.
// Wires in this exam's figure:
//   A = output of Extend
//   B = RD1 (first value read from the register file = value of t2)
//   C = output of the ResultSrc mux (the value written to the register)
//   D = output of the branch adder = PC + immediate
//   E = PC (output of the PC register)
// Key: A = 4, B = 0x003f0011, C = unknown, D = 0x40004050, E = 0x4000404C.
// (The exam asks for "integer numbers"; hex is fine.)
// Why unknown: C is the word read from memory at 0x003f0015, which is not given.
// D is calculated although lw is not a branch: the adder always adds.
void item13a(void) {
    printf("\nItem 13(a): lw t8,4(t2) after lui / ori   (RISC-V)\n");
    resetAll();
    pc = 0x40004044;  pcKnown = 1;

    rvLuiByHand(t2, 0x003f0);                    // lui t2,0x003f0    -> 0x003f0000
    rvWires(rvI(0x011, t2, 6, t2, 0x13));        // ori t2,t2,0x011   -> 0x003f0011
    commit();

    rvWires(rvI(4, t2, 2, t3, 0x03));            // lw  "t8",4(t2)    (t3 stands in for t8)
    printSteps();
    show("A", w_imm, 1, 4, 1);
    show("B", w_rd1, w_rd1Known, 0x003f0011, 1);
    show("C", w_result, w_resultKnown, 0, 0);
    show("D", w_pcTarget, pcKnown, 0x40004050, 1);
    show("E", pc, pcKnown, 0x4000404C, 1);
}


int main(void) {
    // MIPS-era items
    item2a();
    item3b();
    item4a();
    item5a();
    item6b();
    item7a();
    // RISC-V items
    item8a();
    item9a();
    item10a();
    item11a();
    item12b();
    item13a();
    // PNG-only items (MIPS-era)
    extra_Q4_7a();
    extra_Q4_8a();
    extra_Q4_10a();
    extra_Q4_11b();
    extra_Q4_12a();
    return 0;
}
