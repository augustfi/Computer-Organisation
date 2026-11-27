#include <stdio.h>

// =====================================================================
// Module 4: Processor Design  -  PIPELINE HAZARDS
//
// The 5-stage pipeline: every instruction goes through
//   stage 1 F (fetch)      read the instruction
//   stage 2 D (decode)     read the registers
//   stage 3 E (execute)    the ALU calculates
//   stage 4 M (memory)     load / store
//   stage 5 W (writeback)  write the result to the register
// A new instruction starts every clock cycle, so up to 5 are inside at once.
//
// DATA HAZARD: an instruction needs a register that an earlier instruction
// has not written back yet.
//   - The register file writes in the FIRST half of a cycle and reads in the
//     second half. So an instruction 3 or more lines later is safe.
//   - 1 or 2 lines later: hazard.
//   Solutions:
//   - FORWARDING: send the value straight from a later stage to where it is
//     needed. Costs no extra cycle.
//   - STALLING: hold the instruction back one or more cycles. Needed when the
//     value does not exist yet (a load only has it after the memory stage).
//     After the stall the value is forwarded: "stalling + forwarding".
//
// CONTROL HAZARD: after a branch the processor has already fetched the next
// instructions before it knows if the branch is taken. If it is taken, those
// instructions are wrong and are FLUSHED (thrown away). A branch that can
// never be taken causes no control hazard (the exam keys count it that way).
//
// How this file works:
//   Each exam's code is written as plain arrays, one entry per instruction:
//     kind[]  what sort of instruction (ALU, LOAD, STORE, BRANCH)
//     dst[]   the register it writes          (NONE if it writes none)
//     src1[]  first register it reads         (for a store: the BASE register)
//     src2[]  second register it reads        (for a store: the register that is STORED)
//   findHazards() walks through the arrays and reports every data hazard and
//   how many stall cycles it needs. The result is printed beside the key.
//
// The rule inside findHazards() (see the function for the details):
//   stalls = ready + 1 - need - distance      (if negative: 0 = forwarding only)
//
// Items 1-7 and the extra_ items come from the MIPS exams. Hazards work the
// same in MIPS and RISC-V; only the register names differ ($t0 and t0), and
// old MIPS has "branch delay slots", which RISC-V does not have.
//
// Run:  gcc -Wall module4_hazards.c -o hazards && ./hazards
// =====================================================================


// ---------- RISC-V register numbers ----------
enum {
    zero, ra, sp, gp, tp, t0, t1, t2, s0, s1,
    a0, a1, a2, a3, a4, a5, a6, a7,
    s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
    t3, t4, t5, t6
};

char* rvName[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
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

// ---------- Kinds of instruction ----------
enum {
    ALU,        // add, addi, sub, or, xor, slt, sll ...: the result is ready after EXECUTE
    LOAD,       // lw, lb: the result is only ready after MEMORY
    STORE,      // sw, sb: writes no register
    BRANCH      // beq, bne: writes no register
};

#define NONE -1     // "no register here"

// ---------- What findHazards() found (filled in by it) ----------
int foundCount;         // how many data hazards
int foundFrom[20];      // line number of the instruction that WRITES the register (1 = first line)
int foundTo[20];        // line number of the instruction that NEEDS it
int foundReg[20];       // the register
int foundStalls[20];    // stall cycles needed (0 = forwarding alone is enough)


// The data hazard detector.
//   n              number of instructions
//   text[]         the instruction as text, only for printing
//   names[]        mipsName or rvName, only for printing
//   branchInDecode 1 = the branch compares its registers in the DECODE stage
//                  0 = it compares in the EXECUTE stage (with the ALU)
void findHazards(int n, int kind[], int dst[], int src1[], int src2[],
                 char* text[], char* names[], int branchInDecode) {
    foundCount = 0;

    for (int j = 1; j < n; j++) {                // j = the instruction that reads
        for (int s = 0; s < 2; s++) {            // s = 0: its first source, s = 1: its second
            int r;
            if (s == 0) {
                r = src1[j];
            } else {
                r = src2[j];
            }
            if (r == NONE || r == 0) {
                continue;                        // no register, or register zero (never a hazard)
            }
            if (s == 1 && src2[j] == src1[j]) {
                continue;                        // same register twice: report it once
            }

            // Look backwards for the NEAREST earlier instruction that writes r.
            int i = j - 1;
            while (i >= 0 && dst[i] != r) {
                i--;
            }
            if (i < 0) {
                continue;                        // nobody before writes r
            }

            int distance = j - i;                // 1 = the line right after
            if (distance >= 3) {
                continue;                        // already written back: no hazard
            }

            // In which stage is the value READY?  (available at the END of that stage)
            int ready = 3;                       // ALU instruction: after execute
            if (kind[i] == LOAD) {
                ready = 4;                       // load: after memory
            }

            // In which stage does instruction j NEED the value?
            int need = 3;                        // normally in execute (the ALU input)
            if (kind[j] == BRANCH && branchInDecode) {
                need = 2;                        // the branch compares already in decode
            }
            if (kind[j] == STORE && s == 1) {
                need = 4;                        // the stored value is only needed in memory
            }

            // The writer is in stage "ready" during cycle i + ready.
            // The reader is in stage "need" during cycle j + need.
            // Forwarding works if the reader's cycle is at least one later.
            // Each stall moves the reader one cycle later.
            int stalls = ready + 1 - need - distance;
            if (stalls < 0) {
                stalls = 0;
            }

            foundFrom[foundCount] = i + 1;
            foundTo[foundCount] = j + 1;
            foundReg[foundCount] = r;
            foundStalls[foundCount] = stalls;
            foundCount++;

            printf("  data hazard %d-%d: \"%s\" writes %s, \"%s\" needs it -> ",
                   i + 1, j + 1, text[i], names[r], text[j]);
            if (stalls == 0) {
                printf("forwarding (no stall)\n");
            } else {
                printf("stalling %d cycle(s) + forwarding\n", stalls);
            }
        }
    }
    if (foundCount == 0) {
        printf("  no data hazards found\n");
    }
}

// Compare what findHazards() found with the key.
// keyStalls: the stall cycles in the key, or -1 where the key does not say.
void checkHazards(int keyCount, int keyFrom[], int keyTo[], int keyReg[], int keyStalls[]) {
    int same = (foundCount == keyCount);
    for (int k = 0; same && k < keyCount; k++) {
        if (foundFrom[k] != keyFrom[k]) same = 0;
        if (foundTo[k] != keyTo[k]) same = 0;
        if (foundReg[k] != keyReg[k]) same = 0;
        if (keyStalls[k] != -1 && foundStalls[k] != keyStalls[k]) same = 0;
    }
    if (same) {
        printf("  compared with the key: OK\n");
    } else {
        printf("  compared with the key: WRONG\n");
    }
}

// Compare one number with the key.
void checkNumber(char* what, int got, int key) {
    printf("  %s = %d   key = %d   ", what, got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Add up all stall cycles that findHazards() found.
int totalStalls(void) {
    int sum = 0;
    for (int k = 0; k < foundCount; k++) {
        sum = sum + foundStalls[k];
    }
    return sum;
}

// How many wrongly fetched instructions must be flushed when a branch is taken
// (with "predict not taken")? One for every stage the branch has passed before
// the decision: decided in stage 2 (decode) -> 1, in stage 4 (memory) -> 3.
// (Lecture 10: 3 cycles for the first pipeline, 1 cycle after moving the
// comparison to decode.)
int flushPenalty(int decisionStage) {
    return decisionStage - 1;
}

// A 12-bit RISC-V immediate is signed: if bit 11 is 1 the value is negative.
int signExtend12(unsigned int imm12) {
    if (imm12 & 0x800) {
        return (int)imm12 - 4096;
    }
    return (int)imm12;
}


// ===================== Item 1 (c) =====================
// Source: 2016-06-09 Q4.pdf (c)   (also in Q4-Q9.png)
// MIPS-era item (same answer in RISC-V with lb t0,8(s0) / lw s0,8(t0)).
//       lb $t0,8($s0)
//       lw $s0,8($t0)
// Question: which register is in the hazard, and the most efficient solution?
// Assumption: plain 5-stage pipeline.
// Key: register $t0. Solved with forwarding AND stalling.
// Why: lb is a load. $t0 is only known after the memory stage, but the next
// instruction needs it one stage earlier (execute, to make the address).
// ($s0 is no hazard: lb READS $s0 before lw writes it. That is the normal order.)
void item1c(void) {
    printf("\nItem 1(c): lb / lw   (MIPS)\n");
    char* text[] = {"lb $t0,8($s0)", "lw $s0,8($t0)"};
    int kind[] = {LOAD, LOAD};
    int dst[]  = {m_t0, m_s0};
    int src1[] = {m_s0, m_t0};
    int src2[] = {NONE, NONE};
    findHazards(2, kind, dst, src1, src2, text, mipsName, 0);

    int keyFrom[] = {1}, keyTo[] = {2}, keyReg[] = {m_t0}, keyStalls[] = {1};
    printf("  key: register $t0, forwarding and stalling\n");
    checkHazards(1, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 2 (b) =====================
// Source: 2017-01-12 Q4.pdf (b)   (also in Q4-Q4.png)
// MIPS-era item (same answer in RISC-V).
//       addi $t0,$s0,1
//       lw   $t0,-8($s0)
//       sw   $t1,-4($t0)
//       or   $t0,$s0,$t0
// Question: which instructions cause hazards, which kind, which registers, best solution?
// Assumption: plain 5-stage pipeline.
// Key: two answers are accepted:
//   (i)  only ONE data hazard, lw -> sw, register $t0, stalling + forwarding.
//   (ii) read-after-write (data) hazards at both sw and or, both on $t0;
//        the first is solved with stalling + forwarding.
// The key's note: once sw has been stalled one cycle, or comes one cycle later
// too, and by then lw has written $t0 back. So the second hazard goes away by
// itself; that is why (i) is also right.
// This detector does not move later lines after a stall, so it reports (ii).
// (addi -> sw is no hazard: lw writes $t0 again in between, so sw wants lw's value.)
void item2b(void) {
    printf("\nItem 2(b): addi / lw / sw / or   (MIPS)\n");
    char* text[] = {"addi $t0,$s0,1", "lw $t0,-8($s0)", "sw $t1,-4($t0)", "or $t0,$s0,$t0"};
    int kind[] = {ALU,  LOAD, STORE, ALU};
    int dst[]  = {m_t0, m_t0, NONE,  m_t0};
    int src1[] = {m_s0, m_s0, m_t0,  m_s0};     // sw: base register $t0
    int src2[] = {NONE, NONE, m_t1,  m_t0};     // sw: stored register $t1
    findHazards(4, kind, dst, src1, src2, text, mipsName, 0);

    int keyFrom[] = {2, 2}, keyTo[] = {3, 4}, keyReg[] = {m_t0, m_t0}, keyStalls[] = {1, -1};
    printf("  key (ii): data hazards at sw and at or, register $t0, first one stalling + forwarding\n");
    printf("  key (i):  only lw-sw counts, because the stall also cures the second one\n");
    checkHazards(2, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 3 (a) =====================
// Source: 2017-03-16 Q4.pdf (a)   (also in Q4-Q5.png)
// MIPS-era item.
//             addi $t0,$t1,8
//             lw   $t1,4($s0)
//             bne  $s0,$t2,next
//             ori  $s0,$0,0x4
//     next:   andi $t0,$t2,0x33
// $s0 = 0x100020ff before, $t2 is arbitrary.
// Assumption: 5-stage pipeline WITH BRANCH DELAY SLOTS: the instruction right
// after a branch is always executed, taken or not. (Old MIPS. RISC-V has no
// delay slots: there, a taken branch would skip ori and $s0 could keep its old value.)
// Question: i. which kind of hazard may occur?  ii. $s0 afterwards?  iii. is any
//           hazard solved with forwarding?
// Key: i. control hazard.  ii. 0x4.  iii. no.
void item3a(void) {
    printf("\nItem 3(a): addi / lw / bne / ori / andi with delay slots   (MIPS)\n");
    char* text[] = {"addi $t0,$t1,8", "lw $t1,4($s0)", "bne $s0,$t2,next", "ori $s0,$0,0x4", "andi $t0,$t2,0x33"};
    int kind[] = {ALU,  LOAD, BRANCH, ALU,    ALU};
    int dst[]  = {m_t0, m_t1, NONE,   m_s0,   m_t0};
    int src1[] = {m_t1, m_s0, m_s0,   m_zero, m_t2};
    int src2[] = {NONE, NONE, m_t2,   NONE,   NONE};
    findHazards(5, kind, dst, src1, src2, text, mipsName, 0);

    // i. No data hazard, but there is a branch, and $t2 is arbitrary, so the
    //    branch may be taken: a control hazard.
    int branches = 1;
    printf("  i.   data hazards: %d, branches that may be taken: %d -> kind: control hazard   key: control hazard   ",
           foundCount, branches);
    if (foundCount == 0 && branches == 1) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }

    // ii. Run the code both ways: branch taken (1) and not taken (0).
    int delaySlot = 1;                           // this exam: delay slots
    unsigned int s0After[2];
    for (int taken = 0; taken < 2; taken++) {
        unsigned int s0Value = 0x100020ff;       // value before the code
        // ori comes right after the branch. It is skipped only when the
        // branch is taken AND there is no delay slot.
        if (taken == 0 || delaySlot == 1) {
            s0Value = 0 | 0x4;                   // ori $s0,$0,0x4
        }
        s0After[taken] = s0Value;                // andi does not touch $s0
    }
    unsigned int s0Taken = s0After[1];
    unsigned int s0NotTaken = s0After[0];
    printf("  ii.  $s0 if taken = 0x%x, if not taken = 0x%x   key = 0x4   ", s0Taken, s0NotTaken);
    if (s0Taken == 0x4 && s0NotTaken == 0x4) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }

    // iii. Forwarding only solves data hazards, and there are none.
    printf("  iii. forwarding used: ");
    if (foundCount == 0) {
        printf("no   key: no   OK\n");
    } else {
        printf("yes   key: no   WRONG\n");
    }
}

// ===================== Item 4 (b) =====================
// Source: 2017-06-08 Q4.pdf (b)   (also in Q4-Q6.png)
// MIPS-era item (same idea in RISC-V).
//       lw  $s1,20($t0)
//       # Missing instruction
//       xor $s2,$t0,$t3
// Question: pick the missing instruction so that there are TWO data hazards:
//           lw -> missing, and missing -> xor. How is each solved best?
// Assumption: plain 5-stage pipeline.
// Key: addi $t3,$s1,0  (other answers are possible).
//      First hazard: forwarding + stalling. Second hazard: forwarding only.
// Why: the missing instruction must READ $s1 (what lw writes) and WRITE $t3
// (what xor reads; $t0 would not work, lw does not write it).
void item4b(void) {
    printf("\nItem 4(b): choose the missing instruction   (MIPS)\n");
    char* text[] = {"lw $s1,20($t0)", "addi $t3,$s1,0", "xor $s2,$t0,$t3"};
    int kind[] = {LOAD, ALU,  ALU};
    int dst[]  = {m_s1, m_t3, m_s2};
    int src1[] = {m_t0, m_s1, m_t0};
    int src2[] = {NONE, NONE, m_t3};
    printf("  trying the key's instruction: addi $t3,$s1,0\n");
    findHazards(3, kind, dst, src1, src2, text, mipsName, 0);

    int keyFrom[] = {1, 2}, keyTo[] = {2, 3}, keyReg[] = {m_s1, m_t3}, keyStalls[] = {1, 0};
    printf("  key: lw->missing forwarding + stalling, missing->xor forwarding only\n");
    checkHazards(2, keyFrom, keyTo, keyReg, keyStalls);

    // A wrong try, to see the difference: it writes $t3 but does not read $s1.
    char* text2[] = {"lw $s1,20($t0)", "addi $t3,$t0,0", "xor $s2,$t0,$t3"};
    int src1b[] = {m_t0, m_t0, m_t0};
    printf("  a wrong try: addi $t3,$t0,0 (does not read $s1)\n");
    findHazards(3, kind, dst, src1b, src2, text2, mipsName, 0);
    printf("  -> only %d hazard, so that instruction is not an answer\n", foundCount);
}

// ===================== Item 5 (b) =====================
// Source: 2018-01-15 Q4.pdf (b)   (also in Q4-Q1.png)
// MIPS-era item (same answer in RISC-V).
//   1   addi $t1,$zero,0x30
//   2   lw   $t2,0($t1)
//   3   sw   $t2,0xff18($zero)
//   4   addi $t2,$zero,100
// Question: which data hazards, between which lines, which register, best technique?
// Assumption: plain 5-stage pipeline.
// Key: hazard 1: lines 1-2, $t1, forwarding.  Hazard 2: lines 2-3, $t2, forwarding.
// Note on hazard 2: lw is a load, and usually "load, then use" needs a stall.
// But here sw only needs $t2 as the VALUE TO STORE, and that is used in the
// memory stage, one stage later than an ALU input. So lw's data can be
// forwarded from writeback to memory with no stall. (In item 2(b) the loaded
// register was the BASE ADDRESS of sw, needed in execute: there a stall is needed.)
// Line 4 writes $t2 but reads nothing: no hazard.
void item5b(void) {
    printf("\nItem 5(b): addi / lw / sw / addi   (MIPS)\n");
    char* text[] = {"addi $t1,$zero,0x30", "lw $t2,0($t1)", "sw $t2,0xff18($zero)", "addi $t2,$zero,100"};
    int kind[] = {ALU,    LOAD, STORE,  ALU};
    int dst[]  = {m_t1,   m_t2, NONE,   m_t2};
    int src1[] = {m_zero, m_t1, m_zero, m_zero};    // sw: base register $zero
    int src2[] = {NONE,   NONE, m_t2,   NONE};      // sw: stored register $t2
    findHazards(4, kind, dst, src1, src2, text, mipsName, 0);

    int keyFrom[] = {1, 2}, keyTo[] = {2, 3}, keyReg[] = {m_t1, m_t2}, keyStalls[] = {0, 0};
    printf("  key: 1-2 $t1 forwarding, 2-3 $t2 forwarding\n");
    checkHazards(2, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 6 (a) =====================
// Source: 2018-03-15 Q4.pdf (a)   (also in Q4-Q2.png)
// MIPS-era item (same answer in RISC-V).
//             addi $t1,$zero,5
//     foo:    lw   $t0,0x1fb0($zero)
//             sw   $t0,0x2fb0($zero)
//             beq  $t1,$zero,foo
// Question: how many control hazards, how many data hazards, which registers?
// Assumption: the key assumes the beq comparison is done in the execute stage,
// and says the answer is the same if it is done in decode. Both are run below.
// Key: ZERO control hazards, ONE data hazard, only register $t0.
// Why zero control hazards: $t1 is 5, so beq $t1,$zero never jumps.
// Why addi -> beq is no data hazard: beq is 3 lines after addi. addi writes
// $t1 in the first half of the cycle in which beq reads it.
void item6a(void) {
    printf("\nItem 6(a): addi / lw / sw / beq   (MIPS)\n");
    char* text[] = {"addi $t1,$zero,5", "lw $t0,0x1fb0($zero)", "sw $t0,0x2fb0($zero)", "beq $t1,$zero,foo"};
    int kind[] = {ALU,    LOAD,   STORE,  BRANCH};
    int dst[]  = {m_t1,   m_t0,   NONE,   NONE};
    int src1[] = {m_zero, m_zero, m_zero, m_t1};
    int src2[] = {NONE,   NONE,   m_t0,   m_zero};

    printf("  beq compares in execute:\n");
    findHazards(4, kind, dst, src1, src2, text, mipsName, 0);
    int dataExecute = foundCount;
    printf("  beq compares in decode:\n");
    findHazards(4, kind, dst, src1, src2, text, mipsName, 1);
    int dataDecode = foundCount;

    // Control hazard: can the branch be taken? Work out $t1.
    int t1Value = 0 + 5;                         // addi $t1,$zero,5
    int controlHazards = 0;
    if (t1Value == 0) {                          // beq $t1,$zero jumps only when $t1 is 0
        controlHazards = 1;
    }
    printf("  $t1 = %d, so the branch is never taken\n", t1Value);

    checkNumber("control hazards", controlHazards, 0);
    checkNumber("data hazards (execute)", dataExecute, 1);
    checkNumber("data hazards (decode)", dataDecode, 1);
    printf("  register: %s   key: $t0   ", mipsName[foundReg[0]]);
    if (foundCount == 1 && foundReg[0] == m_t0) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
    // The key does not ask how the lw-sw hazard is solved. By the reasoning of
    // item 5(b) (same pattern, the loaded register is the stored value): forwarding.
}

// ===================== Item 7 (b) =====================
// Source: 2018-06-05 Q4.pdf (b)   (also in Q4-Q3.png)
// MIPS-era item (same answer in RISC-V).
//             lw  $t1,4($s0)
//             slt $t1,$s0,$t2
//             beq $t1,$s1,next
//             add $t1,$s0,$s1
//     next:
// Question: which data hazards, which instructions and registers, how solved?
// Assumption: the beq comparison is done in the DECODE stage.
// Key: only ONE data hazard, slt -> beq, register $t1.
//      Solved by stalling one clock cycle, then forwarding.
// Why a stall although slt is no load: beq needs $t1 already in decode, one
// stage earlier than usual. slt is still in execute at that moment.
// lw -> beq is no hazard: slt writes $t1 again in between, so beq wants slt's value.
void item7b(void) {
    printf("\nItem 7(b): lw / slt / beq / add, beq decided in decode   (MIPS)\n");
    char* text[] = {"lw $t1,4($s0)", "slt $t1,$s0,$t2", "beq $t1,$s1,next", "add $t1,$s0,$s1"};
    int kind[] = {LOAD, ALU,  BRANCH, ALU};
    int dst[]  = {m_t1, m_t1, NONE,   m_t1};
    int src1[] = {m_s0, m_s0, m_t1,   m_s0};
    int src2[] = {NONE, m_t2, m_s1,   m_s1};
    findHazards(4, kind, dst, src1, src2, text, mipsName, 1);

    int keyFrom[] = {2}, keyTo[] = {3}, keyReg[] = {m_t1}, keyStalls[] = {1};
    printf("  key: slt-beq, $t1, stall one cycle then forward\n");
    checkHazards(1, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 8 (b) =====================
// Source: 2019-01-12-riscv Q4.pdf (b)
// RISC-V.
//             lw   t0,0x708(zero)
//             beq  t0,zero,foo
//             addi t0,zero,5
//     foo:    addi t0,zero,10
// Assumption: beq compares in the DECODE stage, static "predict not taken".
// Question: i. how many data and control hazards?  ii. which instructions are in
//           the data hazard?  iii. the largest total penalty in clock cycles?
// Key: i. one data hazard, one control hazard.  ii. lw and beq.  iii. 3 cycles
//      (2 stall cycles before lw's memory result can reach the decode stage of
//       beq, plus 1 cycle if the branch is taken).
void item8b(void) {
    printf("\nItem 8(b): lw / beq / addi / addi, beq in decode, predict not taken   (RISC-V)\n");
    char* text[] = {"lw t0,0x708(zero)", "beq t0,zero,foo", "addi t0,zero,5", "addi t0,zero,10"};
    int kind[] = {LOAD, BRANCH, ALU,  ALU};
    int dst[]  = {t0,   NONE,   t0,   t0};
    int src1[] = {zero, t0,     zero, zero};
    int src2[] = {NONE, zero,   NONE, NONE};
    findHazards(4, kind, dst, src1, src2, text, rvName, 1);

    // Control hazard: t0 comes from memory, we do not know it, so the branch
    // MAY be taken: one control hazard.
    int controlHazards = 1;
    checkNumber("i.   data hazards", foundCount, 1);
    checkNumber("i.   control hazards", controlHazards, 1);

    int keyFrom[] = {1}, keyTo[] = {2}, keyReg[] = {t0}, keyStalls[] = {2};
    printf("  ii.  key: lw and beq\n");
    checkHazards(1, keyFrom, keyTo, keyReg, keyStalls);

    // Worst case: the stall cycles, plus the flush when the guess "not taken" was wrong.
    int penalty = totalStalls() + controlHazards * flushPenalty(2);   // 2 = decode stage
    checkNumber("iii. largest penalty in cycles", penalty, 3);
}

// ===================== Item 9 (b) =====================
// Source: 2019-03-15-riscv Q4.pdf (b)
// RISC-V.
//   1         addi t0,zero,0x720
//   2         sll  t2,t0,16
//   3         addi t3,zero,0xf
//   4         lw   t1,8(t2)
//   5         bne  t0,zero,foo
//   6         xori t1,zero,1
//     foo:
// Question: for each data hazard: between which instructions, which register,
//           best solution (forwarding, stalling, or both)?
// Assumption: plain 5-stage pipeline. The exam does not say where the branch
// is decided, and it does not matter: bne is 4 lines after the addi that
// writes t0, so t0 is long since written back.
// Key: two hazards. addi(1) -> sll(2), register t0, forwarding.
//                   sll(2) -> lw(4),  register t2, forwarding.
// (The exam writes sll with a constant; in real RISC-V that is slli.)
void item9b(void) {
    printf("\nItem 9(b): addi / sll / addi / lw / bne / xori   (RISC-V)\n");
    char* text[] = {"addi t0,zero,0x720", "sll t2,t0,16", "addi t3,zero,0xf",
                    "lw t1,8(t2)", "bne t0,zero,foo", "xori t1,zero,1"};
    int kind[] = {ALU,  ALU,  ALU,  LOAD, BRANCH, ALU};
    int dst[]  = {t0,   t2,   t3,   t1,   NONE,   t1};
    int src1[] = {zero, t0,   zero, t2,   t0,     zero};
    int src2[] = {NONE, NONE, NONE, NONE, zero,   NONE};
    findHazards(6, kind, dst, src1, src2, text, rvName, 1);

    int keyFrom[] = {1, 2}, keyTo[] = {2, 4}, keyReg[] = {t0, t2}, keyStalls[] = {0, 0};
    printf("  key: 1-2 t0 forwarding, 2-4 t2 forwarding\n");
    checkHazards(2, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 10 (b) =====================
// Source: 2019-06-05-riscv Q4.pdf (b)
// RISC-V registers, but with an old-MIPS assumption:
//   1         addi t0,zero,8
//   2         lw   t0,-8(zero)
//   3         beq  t0,zero,skip
//   4         addi t0,zero,5
//   5         add  t0,t2,t1
//     skip:
// Assumption: the processor uses BRANCH DELAY SLOTS (the instruction after the
// branch always runs) and the comparison is done in the DECODE stage.
// (Real RISC-V has no delay slots. Follow what the exam text says.)
// Question: i. between which instructions is the data hazard?  ii. is stalling
//           needed, how many cycles?  iii. if the branch is taken, what is t0 at skip?
// Key: i. between lw and beq.  ii. yes, 2 clock cycles.
//      iii. 5: the addi in the delay slot runs after beq although the branch is taken.
void item10b(void) {
    printf("\nItem 10(b): addi / lw / beq / addi / add, delay slots, beq in decode   (RISC-V)\n");
    char* text[] = {"addi t0,zero,8", "lw t0,-8(zero)", "beq t0,zero,skip", "addi t0,zero,5", "add t0,t2,t1"};
    int kind[] = {ALU,  LOAD, BRANCH, ALU,  ALU};
    int dst[]  = {t0,   t0,   NONE,   t0,   t0};
    int src1[] = {zero, zero, t0,     zero, t2};
    int src2[] = {NONE, NONE, zero,   NONE, t1};
    findHazards(5, kind, dst, src1, src2, text, rvName, 1);

    int keyFrom[] = {2}, keyTo[] = {3}, keyReg[] = {t0}, keyStalls[] = {2};
    printf("  i.   key: between lw and beq\n");
    checkHazards(1, keyFrom, keyTo, keyReg, keyStalls);
    checkNumber("ii.  stall cycles", totalStalls(), 2);

    // iii. Run the code with the branch TAKEN and a delay slot.
    int taken = 1;
    int delaySlot = 1;
    int value;
    value = 0 + 8;               // 1: addi t0,zero,8
    value = 0;                   // 2: lw: not given, but the branch is taken, so it must have loaded 0
    // 3: beq. Line 4 comes right after the branch: it is skipped only when the
    //    branch is taken AND there is no delay slot.
    if (taken == 0 || delaySlot == 1) {
        value = 0 + 5;           // 4: addi t0,zero,5
    }
    // 5: add t0,t2,t1 is jumped over when the branch is taken.
    checkNumber("iii. t0 at skip", value, 5);
}

// ===================== Item 11 (b) =====================
// Source: 2020-01-13-riscv Q4.pdf (b)
// RISC-V.
//   1         addi t1,zero,1
//   2         addi t2,t2,0
//   3  loop:  lb   t0,0(s0)
//   4         add  t2,t0,t2
//   5         addi t1,t1,-1
//   6         addi s0,s0,1
//   7         bne  t1,zero,loop
// Assumption: the equality check of the branch is done in the DECODE stage.
// Question: i. list all data hazards as "writer-reader".  ii. which need stalling?
// Key: i. 2-4, 3-4, 5-7.   ii. 3-4.
// Why 5-7 needs no stall although the branch is decided in decode: there is one
// line (6) in between, so line 5 has finished execute when line 7 is in decode.
// Why 1-5 is not on the list: 4 lines apart, t1 is already written back.
// (t1 starts at 1, so the loop body runs once and bne is not taken.)
void item11b(void) {
    printf("\nItem 11(b): the 7-instruction loop, branch in decode   (RISC-V)\n");
    char* text[] = {"addi t1,zero,1", "addi t2,t2,0", "lb t0,0(s0)", "add t2,t0,t2",
                    "addi t1,t1,-1", "addi s0,s0,1", "bne t1,zero,loop"};
    int kind[] = {ALU,  ALU,  LOAD, ALU, ALU,  ALU,  BRANCH};
    int dst[]  = {t1,   t2,   t0,   t2,  t1,   s0,   NONE};
    int src1[] = {zero, t2,   s0,   t0,  t1,   s0,   t1};
    int src2[] = {NONE, NONE, NONE, t2,  NONE, NONE, zero};
    findHazards(7, kind, dst, src1, src2, text, rvName, 1);

    // The key's three hazards, in the order the detector reports them.
    int keyFrom[] = {3, 2, 5}, keyTo[] = {4, 4, 7}, keyReg[] = {t0, t2, t1}, keyStalls[] = {1, 0, 0};
    printf("  key: i. 2-4, 3-4, 5-7   ii. only 3-4 needs stalling\n");
    checkHazards(3, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 12 (a) =====================
// Source: 2020-03-11-riscv Q4.pdf (a)
// RISC-V. Four  sll s4,s4,0  lines come first; the exam says to leave them out.
// (They only make sure that nothing earlier is still in the pipeline.)
//   1         addi s1,zero,0x7f4
//   2         sw   t0,4(s1)
//   3         beq  t0,t1,skip
//   4         lw   t0,4(s2)
//     skip:
// Assumption: 5-stage pipeline WITHOUT branch delay slots.
// Question: for each data hazard: which instructions, which register, and the
//           solution with the lowest penalty?
// Key: only ONE data hazard: between addi and sw, register s1, forwarding.
// Why nothing else: beq reads t0 and t1, and nobody before writes them
// (sw only READS t0). lw writes t0 AFTER beq read it: that is the normal order.
void item12a(void) {
    printf("\nItem 12(a): addi / sw / beq / lw, no delay slots   (RISC-V)\n");
    char* text[] = {"addi s1,zero,0x7f4", "sw t0,4(s1)", "beq t0,t1,skip", "lw t0,4(s2)"};
    int kind[] = {ALU,  STORE, BRANCH, LOAD};
    int dst[]  = {s1,   NONE,  NONE,   t0};
    int src1[] = {zero, s1,    t0,     s2};     // sw: base register s1
    int src2[] = {NONE, t0,    t1,     NONE};   // sw: stored register t0
    findHazards(4, kind, dst, src1, src2, text, rvName, 1);

    int keyFrom[] = {1}, keyTo[] = {2}, keyReg[] = {s1}, keyStalls[] = {0};
    printf("  key: addi-sw, s1, forwarding\n");
    checkHazards(1, keyFrom, keyTo, keyReg, keyStalls);
}

// ===================== Item 13 (b) =====================
// Source: 2020-06-05-riscv Q4.pdf (b)
// RISC-V.
//             addi t1,zero,0x821
//             addi t2,zero,0xb21
//             addi t0,zero,0x18
//     foo:    lw   t0,0x6b0(zero)
//             beq  zero,t1,foo
// Question: how many control hazards, how many data hazards, which registers?
// Assumption: plain 5-stage pipeline (the key's answer holds for branch in
// decode and in execute).
// Key: ZERO control hazards and ZERO data hazards.
// Why: t1 is not 0, so beq zero,t1 is never taken: no control hazard.
// beq is 4 lines after the addi that writes t1: the value is already in the
// register file. lw writes t0 but nobody reads t0.
// (0x821 and 0xb21 do not fit as positive 12-bit immediates. As 12-bit patterns
// they are negative. Either way t1 is not zero, so the answer is the same.)
void item13b(void) {
    printf("\nItem 13(b): three addi, then lw / beq   (RISC-V)\n");
    char* text[] = {"addi t1,zero,0x821", "addi t2,zero,0xb21", "addi t0,zero,0x18",
                    "lw t0,0x6b0(zero)", "beq zero,t1,foo"};
    int kind[] = {ALU,  ALU,  ALU,  LOAD, BRANCH};
    int dst[]  = {t1,   t2,   t0,   t0,   NONE};
    int src1[] = {zero, zero, zero, zero, zero};
    int src2[] = {NONE, NONE, NONE, NONE, t1};
    findHazards(5, kind, dst, src1, src2, text, rvName, 1);

    // Control hazard: can the branch be taken? Work out t1.
    int t1Value = 0 + signExtend12(0x821);       // addi t1,zero,0x821
    int controlHazards = 0;
    if (t1Value == 0) {                          // beq zero,t1 jumps only when t1 is 0
        controlHazards = 1;
    }
    printf("  t1 = %d (0x%x), not 0, so the branch is never taken\n", t1Value, (unsigned int)t1Value);

    checkNumber("control hazards", controlHazards, 0);
    checkNumber("data hazards", foundCount, 0);
}

// ===================== extra_Q4_7 (b) =====================
// Source: Q4-Q7.png (b)   (answer is in the capture; belongs to none of the 13 sittings)
// MIPS-era item (same answer in RISC-V, where flushing is the normal way).
//             add  $t0,$t1,$t2
//             lw   $t1,-4($t2)
//             beq  $t2,$zero,skip
//             addi $t0,$zero,1
//     skip:
// All values are in the register file when add is fetched.
// Assumption: plain 5-stage pipeline, no delay slots.
// Question: which instruction(s) can cause hazards, which kind, how handled?
// Key: only beq. A CONTROL hazard, if the branch is taken. Handled by flushing
//      the pipeline and fetching the right instruction.
// Why no data hazard: nobody reads a register that an earlier line writes.
// (lw writes $t1 AFTER add read it. The key calls that a write-after-read
//  hazard and says it only matters in an out-of-order processor, not here.)
void extra_Q4_7b(void) {
    printf("\nextra_Q4_7(b): add / lw / beq / addi   (MIPS)\n");
    char* text[] = {"add $t0,$t1,$t2", "lw $t1,-4($t2)", "beq $t2,$zero,skip", "addi $t0,$zero,1"};
    int kind[] = {ALU,  LOAD, BRANCH, ALU};
    int dst[]  = {m_t0, m_t1, NONE,   m_t0};
    int src1[] = {m_t1, m_t2, m_t2,   m_zero};
    int src2[] = {m_t2, NONE, m_zero, NONE};
    findHazards(4, kind, dst, src1, src2, text, mipsName, 0);

    // $t2 is not given, so the branch may be taken: one control hazard.
    int controlHazards = 1;
    checkNumber("data hazards", foundCount, 0);
    checkNumber("control hazards (at beq)", controlHazards, 1);
    printf("  handled by: flushing   key: flushing\n");
}

// ===================== extra_Q4_8 (b) =====================
// Source: Q4-Q8.png (b)   (answer is in the capture; belongs to none of the 13 sittings)
// MIPS-era item (same answer in RISC-V: lw t0,0(t1)).
//       # Missing instruction
//       addi $t2,$t0,1
// Question: pick the missing instruction so that there is a data hazard that
//           needs forwarding AND stalling.
// Assumption: plain 5-stage pipeline.
// Key: lw $t0,0($t1)   (a load, lw or lb, that writes $t0; any base register).
void extra_Q4_8b(void) {
    printf("\nextra_Q4_8(b): missing instruction before addi $t2,$t0,1   (MIPS)\n");
    char* text[] = {"lw $t0,0($t1)", "addi $t2,$t0,1"};
    int kind[] = {LOAD, ALU};
    int dst[]  = {m_t0, m_t2};
    int src1[] = {m_t1, m_t0};
    int src2[] = {NONE, NONE};
    printf("  trying the key's instruction: lw $t0,0($t1)\n");
    findHazards(2, kind, dst, src1, src2, text, mipsName, 0);

    int keyFrom[] = {1}, keyTo[] = {2}, keyReg[] = {m_t0}, keyStalls[] = {1};
    printf("  key: a load that writes $t0 -> forwarding + stalling\n");
    checkHazards(1, keyFrom, keyTo, keyReg, keyStalls);

    // A wrong try: an ALU instruction that writes $t0. Hazard, but forwarding is enough.
    char* text2[] = {"addi $t0,$t1,0", "addi $t2,$t0,1"};
    int kind2[] = {ALU, ALU};
    printf("  a wrong try: addi $t0,$t1,0\n");
    findHazards(2, kind2, dst, src1, src2, text2, mipsName, 0);
    printf("  -> %d stall cycle(s), so no stalling: not an answer\n", foundStalls[0]);
}

// ===================== extra_Q4_10 (c) =====================
// Source: Q4-Q10.png (c)   (answer is in the capture; the header says "Module 3",
//         the numbering of an older year)
// MIPS-era item (same answer in RISC-V).
// Question: the processor uses "branch not taken". Which kind of hazard may beq
//           cause, and what is the earliest stage where the misprediction can be
//           detected, if the misprediction penalty is 1 clock cycle?
// Key: a control hazard. Detected in the decode stage.
void extra_Q4_10c(void) {
    printf("\nextra_Q4_10(c): where is the branch decided if the penalty is 1 cycle?\n");
    char* stageName[] = {"", "fetch", "decode", "execute", "memory", "writeback"};
    int answer = 0;
    // Try each stage: how many wrongly fetched instructions would have to be flushed?
    for (int stage = 2; stage <= 4; stage++) {
        int penalty = flushPenalty(stage);
        printf("  decided in %s (stage %d): penalty %d cycle(s)\n", stageName[stage], stage, penalty);
        if (penalty == 1) {
            answer = stage;
        }
    }
    printf("  kind of hazard: control hazard   key: control hazard\n");
    printf("  stage = %s   key = decode   ", stageName[answer]);
    if (answer == 2) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// ===================== extra_Q4_11 (a) =====================
// Source: Q4-Q11.png (a)   (answer is in the capture; header says "Module 3")
// MIPS-era item (same answer in RISC-V).
//       addi $s0,$t0,8
//       sw   $t0,8($s0)
//       sub  $s0,$s0,$t1
// Question: i) which instructions and which register cause a hazard?  ii) data or
//           control?  iii) best solution?  iv) the cost in extra clock cycles?
// Assumption: plain 5-stage pipeline.
// Key: i) addi, sw and sub; register $s0 (its new value is not in the register
//      file yet when sw and sub need it).  ii) data hazards.  iii) forwarding.
//      iv) no extra clock cycles.
void extra_Q4_11a(void) {
    printf("\nextra_Q4_11(a): addi / sw / sub   (MIPS)\n");
    char* text[] = {"addi $s0,$t0,8", "sw $t0,8($s0)", "sub $s0,$s0,$t1"};
    int kind[] = {ALU,  STORE, ALU};
    int dst[]  = {m_s0, NONE,  m_s0};
    int src1[] = {m_t0, m_s0,  m_s0};       // sw: base register $s0
    int src2[] = {NONE, m_t0,  m_t1};       // sw: stored register $t0
    findHazards(3, kind, dst, src1, src2, text, mipsName, 0);

    int keyFrom[] = {1, 1}, keyTo[] = {2, 3}, keyReg[] = {m_s0, m_s0}, keyStalls[] = {0, 0};
    printf("  key: addi-sw and addi-sub, $s0, data hazards, forwarding\n");
    checkHazards(2, keyFrom, keyTo, keyReg, keyStalls);
    checkNumber("extra clock cycles", totalStalls(), 0);
}


int main(void) {
    // MIPS-era items
    item1c();
    item2b();
    item3a();
    item4b();
    item5b();
    item6a();
    item7b();
    // RISC-V items
    item8b();
    item9b();
    item10b();
    item11b();
    item12a();
    item13b();
    // PNG-only items (MIPS-era)
    extra_Q4_7b();
    extra_Q4_8b();
    extra_Q4_10c();
    extra_Q4_11a();
    return 0;
}
