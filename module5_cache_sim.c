#include <stdio.h>

// =====================================================================
// Module 5: MEMORY HIERARCHY, part 2: HIT or MISS
//
// Every exam item is its own function (item1a, item2b, ...).
// There is ONE small cache simulator at the top. Each item
//   1. sets up the cache(s) with the numbers from the question,
//   2. feeds the simulator every address the program touches, in order:
//        - the address of each instruction that is fetched (instruction cache)
//        - the address of each load/store (data cache)
//   3. prints every access as hit or miss, then the rate next to the exam's
//      answer ("key").
//
// Items 1 to 7 and the extras come from old MIPS exams. A cache only sees
// ADDRESSES, so the addresses and answers are the same for RISC-V. Where a
// MIPS instruction is quoted, its RISC-V form is written next to it.
//
// Run:  gcc -Wall module5_cache_sim.c -o sim && ./sim
//
// ---------------------------------------------------------------------
// How a cache decides hit or miss
//
//   address = | tag | set | byte offset |
//
//   block number = address / block size      (drops the byte offset bits)
//   set          = block number % sets       (the lowest bits of the block number)
//   tag          = block number / sets       (the rest of the bits)
//
//   HIT  = some way in that set has valid = 1 AND the same tag.
//   MISS = otherwise. The whole block is loaded: valid = 1, tag = new tag.
//
// Two kinds of locality (words used by the exam key):
//   temporal = the SAME address is used again and is still in the cache.
//   spatial  = an address is a hit the FIRST time it is used, because a
//              NEIGHBOUR in the same block was loaded earlier.
// The simulator counts both kinds of hit, so the locality answer is computed.
// =====================================================================


// ---------- Sizes of the simulator's arrays ----------
#define CACHES 2          // we simulate two caches at the same time
#define MAX_SETS 4096     // the biggest number of sets in any exam item
#define MAX_WAYS 4
#define MAX_ACCESSES 1000 // the longest program in any exam item touches 403 addresses

// Names for the two caches, used as the first index of every array below.
// An enum counts from 0, so ICACHE = 0 and DCACHE = 1.
enum { ICACHE, DCACHE };
char* cacheName[CACHES] = { "I-cache", "D-cache" };

// ---------- The shape of each cache ----------
int blockSize[CACHES];    // bytes per block
int sets[CACHES];         // number of sets (rows)
int ways[CACHES];         // blocks per set (1 = direct mapped)

// ---------- The contents of each cache: [cache][set][way] ----------
int valid[CACHES][MAX_SETS][MAX_WAYS];           // 1 = this block holds real data
unsigned int tags[CACHES][MAX_SETS][MAX_WAYS];   // which block is stored here
int lastUsed[CACHES][MAX_SETS][MAX_WAYS];        // time of the latest use (for LRU)

// ---------- Counters ----------
int clockNow[CACHES];       // counts accesses; used as the "time" for LRU
int hits[CACHES];
int misses[CACHES];
int temporalHits[CACHES];   // hits on an address that was used before
int spatialHits[CACHES];    // hits on an address that is used for the first time

// Every address seen so far (to tell temporal from spatial), and the result
// of every access as a letter: 'H' = hit, 'M' = miss.
unsigned int seen[CACHES][MAX_ACCESSES];
char result[CACHES][MAX_ACCESSES + 1];
int accesses[CACHES];

// Set show = 0 to stop printing each access (used inside very long loops).
int show = 1;


// ---------- The simulator ----------

// Give cache c its shape and empty it (all valid bits = 0).
// Formula:  sets = capacity / (block size x ways)
void setupCache(int c, int capacity, int blockBytes, int numberOfWays) {
    blockSize[c] = blockBytes;
    ways[c] = numberOfWays;
    sets[c] = capacity / (blockBytes * numberOfWays);

    for (int s = 0; s < sets[c]; s++) {
        for (int w = 0; w < ways[c]; w++) {
            valid[c][s][w] = 0;
            tags[c][s][w] = 0;
            lastUsed[c][s][w] = 0;
        }
    }
    clockNow[c] = 0;
    hits[c] = 0;
    misses[c] = 0;
    temporalHits[c] = 0;
    spatialHits[c] = 0;
    accesses[c] = 0;
    result[c][0] = '\0';

    printf("  %s: %d bytes, block %d bytes, %d-way  ->  %d sets\n",
           cacheName[c], capacity, blockBytes, numberOfWays, sets[c]);
}

// Use one address in cache c. Returns 1 for a hit and 0 for a miss.
// label is only text for the printout (the instruction's name).
int cacheAccess(int c, unsigned int address, char* label) {
    // Step 1: cut the address into its fields.
    unsigned int blockNumber = address / blockSize[c];   // drop the byte offset
    unsigned int set = blockNumber % sets[c];            // lowest bits = set number
    unsigned int tag = blockNumber / sets[c];            // the rest = tag

    clockNow[c]++;                                       // time moves one step

    // Step 2: was this exact address used before? (only needed for the locality answer)
    int usedBefore = 0;
    for (int i = 0; i < accesses[c]; i++) {
        if (seen[c][i] == address) {
            usedBefore = 1;
        }
    }

    // Step 3: look in every way of the set for a valid block with the same tag.
    int hit = 0;
    for (int w = 0; w < ways[c]; w++) {
        if (valid[c][set][w] == 1 && tags[c][set][w] == tag) {
            hit = 1;
            lastUsed[c][set][w] = clockNow[c];           // remember that it was just used
        }
    }

    // Step 4: on a miss, pick the way to load the new block into.
    if (hit == 0) {
        int victim = -1;
        // First choice: a way that is empty (valid = 0).
        for (int w = 0; w < ways[c]; w++) {
            if (victim == -1 && valid[c][set][w] == 0) {
                victim = w;
            }
        }
        // Otherwise LRU: the way that was used the longest time ago.
        // (In a direct mapped cache there is only way 0, so it is always replaced.)
        if (victim == -1) {
            victim = 0;
            for (int w = 1; w < ways[c]; w++) {
                if (lastUsed[c][set][w] < lastUsed[c][set][victim]) {
                    victim = w;
                }
            }
        }
        // The three things that are updated on a miss:
        valid[c][set][victim] = 1;                       // 1. the valid bit
        tags[c][set][victim] = tag;                      // 2. the tag
        lastUsed[c][set][victim] = clockNow[c];          // 3. the block data (not stored in this simulator)
    }

    // Step 5: count, remember, print.
    char* kind = "";
    if (hit == 1) {
        hits[c]++;
        if (usedBefore == 1) {
            temporalHits[c]++;
            kind = "(temporal: same address again)";
        } else {
            spatialHits[c]++;
            kind = "(spatial: neighbour loaded the block)";
        }
    } else {
        misses[c]++;
    }

    if (accesses[c] < MAX_ACCESSES) {
        seen[c][accesses[c]] = address;
        if (hit == 1) {
            result[c][accesses[c]] = 'H';
        } else {
            result[c][accesses[c]] = 'M';
        }
        accesses[c]++;
        result[c][accesses[c]] = '\0';                   // keep it a proper C string
    }

    if (show == 1) {
        printf("    %s 0x%08x %-5s set 0x%03x tag 0x%06x  ", cacheName[c], address, label, set, tag);
        if (hit == 1) {
            printf("hit  %s\n", kind);
        } else {
            printf("MISS\n");
        }
    }
    return hit;
}


// ---------- Helpers for printing answers ----------

// Print a rate as a fraction and in percent, next to the key's fraction.
// Two fractions a/b and c/d are equal when a*d == c*b (no rounding needed).
// Use keyBottom = 0 when the exam gives no answer to compare with.
void checkRate(char* what, int top, int bottom, int keyTop, int keyBottom) {
    printf("  %s = %d/%d = %.1f%%", what, top, bottom, 100.0 * top / bottom);
    if (keyBottom == 0) {
        printf("   (no key: not asked in the exam)\n");
    } else if (top * keyBottom == keyTop * bottom) {
        printf("   key = %d/%d   OK\n", keyTop, keyBottom);
    } else {
        printf("   key = %d/%d   WRONG\n", keyTop, keyBottom);
    }
}

void checkHitRate(int c, int keyTop, int keyBottom) {
    printf("  %s", cacheName[c]);
    checkRate(" hit rate", hits[c], hits[c] + misses[c], keyTop, keyBottom);
}

void checkMissRate(int c, int keyTop, int keyBottom) {
    printf("  %s", cacheName[c]);
    checkRate(" miss rate", misses[c], hits[c] + misses[c], keyTop, keyBottom);
}

// Compare the first accesses of cache c with the key's list.
// key is a string like "MMHMH": M = miss, H = hit, in the order of the accesses.
void checkPattern(int c, char* key) {
    int same = 1;
    int i = 0;
    printf("  %s first accesses: ", cacheName[c]);
    while (key[i] != '\0') {
        printf("%c", result[c][i]);
        if (result[c][i] != key[i]) {
            same = 0;
        }
        i++;
    }
    printf("   key = %s   ", key);
    if (same == 1) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Which locality did cache c show? Codes for the key:
enum { NEITHER, TEMPORAL, SPATIAL, BOTH, NO_KEY };
char* localityName[] = { "neither", "temporal only", "spatial only", "both temporal and spatial", "" };

void checkLocality(int c, int key) {
    int got = NEITHER;
    if (temporalHits[c] > 0 && spatialHits[c] > 0) {
        got = BOTH;
    } else if (temporalHits[c] > 0) {
        got = TEMPORAL;
    } else if (spatialHits[c] > 0) {
        got = SPATIAL;
    }
    printf("  %s locality: %d temporal hits, %d spatial hits -> %s",
           cacheName[c], temporalHits[c], spatialHits[c], localityName[got]);
    if (key == NO_KEY) {
        printf("   (no key: not asked in the exam)\n");
    } else if (got == key) {
        printf("   key = %s   OK\n", localityName[key]);
    } else {
        printf("   key = %s   WRONG\n", localityName[key]);
    }
}

// Print one set of cache c: the valid bit and tag of every way.
// The exam calls the ways "way 1" and "way 2"; in the arrays they are 0 and 1.
void printSet(int c, int set) {
    for (int w = 0; w < ways[c]; w++) {
        printf("    set 0x%02x way %d: valid = %d, tag = 0x%05x\n", set, w + 1, valid[c][set][w], tags[c][set][w]);
    }
}


// ===================== Item 1 (a) =====================
// 2016-06-09 Q5.pdf
// Direct mapped instruction cache, 4096 bytes, block 8 bytes, empty at start.
// Six instructions run in a row, starting at 0x4000ff04. Hit rate?
// Key: 1/3 (miss, miss, hit, miss, hit, miss)
void item1a(void) {
    printf("\nItem 1(a): six instructions in a row from 0x4000ff04\n");
    setupCache(ICACHE, 4096, 8, 1);

    unsigned int pc = 0x4000ff04;                // address of the first instruction
    for (int i = 0; i < 6; i++) {
        cacheAccess(ICACHE, pc, "instr");        // fetch the instruction
        pc = pc + 4;                             // the next instruction is 4 bytes further
    }
    // The first instruction is the SECOND word of its block, so its neighbour
    // (at 0x4000ff00) is never used. That is why there are 4 misses, not 3.
    checkPattern(ICACHE, "MMHMHM");
    checkHitRate(ICACHE, 1, 3);
}

// ===================== Item 2 (b) =====================
// 2017-01-12 Q5.pdf
// Data cache: direct mapped, 1024 bytes. Instruction cache: 2-way, 4096 bytes.
// Block size 8 bytes in both. lui is at 0x20004000. Both caches empty at start.
//
//   MIPS in the exam                RISC-V form
//         lui  $s0,0x1234                 lui  s0,0x12342     (see note)
//         ori  $s0,0x2000                 ori  s0,s0,0        (see note)
//         addi $t0,$zero,20               addi t0,zero,20
//   loop: add  $t1,$t0,$s0          loop: add  t1,t0,s0
//         lw   $t2,-4($t1)                lw   t2,-4(t1)
//         addi $t0,$t0,-4                 addi t0,t0,-4
//         bne  $t0,$zero,loop             bne  t0,zero,loop
//
// Note: MIPS lui fills the top 16 bits and ori takes a 16-bit number, so
// s0 = 0x12342000. In RISC-V lui fills the top 20 bits, so the same value is
// just  lui s0,0x12342. The ori then has nothing left to do, but we keep it
// (three instructions before the loop) so that every instruction has the same
// address as in the exam.
//
// Data cache hit rate? Instruction cache hit rate?
// Key: data 40% (miss, miss, hit, miss, hit), instruction 19/23
void item2b(void) {
    printf("\nItem 2(b): loop with lw t2,-4(t1), t0 = 20, 16, 12, 8, 4\n");
    setupCache(DCACHE, 1024, 8, 1);
    setupCache(ICACHE, 4096, 8, 2);

    unsigned int pc = 0x20004000;
    cacheAccess(ICACHE, pc, "lui");   pc = pc + 4;
    cacheAccess(ICACHE, pc, "ori");   pc = pc + 4;
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    unsigned int loop = pc;                      // address of the label "loop"

    unsigned int s0 = 0x12342000;                // lui + ori
    unsigned int t0 = 20;                        // addi t0,zero,20
    unsigned int t1;

    while (t0 != 0) {                            // bne t0,zero,loop
        pc = loop;                               // jump back to the first loop instruction
        cacheAccess(ICACHE, pc, "add");   pc = pc + 4;
        t1 = t0 + s0;                            // add t1,t0,s0

        cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
        cacheAccess(DCACHE, t1 - 4, "lw");       // lw t2,-4(t1): reads address t1 - 4

        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t0 = t0 - 4;                             // addi t0,t0,-4

        cacheAccess(ICACHE, pc, "bne");   pc = pc + 4;
    }

    checkPattern(DCACHE, "MMHMH");
    checkHitRate(DCACHE, 2, 5);                  // 40% = 2/5
    checkHitRate(ICACHE, 19, 23);
}

// ===================== Item 3 (c) =====================
// 2017-03-16 Q5.pdf
// Direct mapped instruction cache, 2048 bytes, block 16 bytes, empty at start.
// A loop of 10 instructions starts at 0x00400200 and runs 10 times. Hit rate?
// Key: 97% (first round: miss, hit, hit, hit, miss, hit, hit, hit, miss, hit)
void item3c(void) {
    printf("\nItem 3(c): loop of 10 instructions from 0x00400200, 10 rounds\n");
    setupCache(ICACHE, 2048, 16, 1);

    for (int round = 0; round < 10; round++) {
        show = (round < 2);                      // print only the first two rounds
        unsigned int pc = 0x00400200;            // every round starts at the first loop instruction
        for (int i = 0; i < 10; i++) {
            cacheAccess(ICACHE, pc, "instr");
            pc = pc + 4;
        }
    }
    show = 1;
    printf("    ... rounds 3 to 10 are all hits, like round 2\n");

    checkPattern(ICACHE, "MHHHMHHHMH");
    checkHitRate(ICACHE, 97, 100);
}

// ===================== Item 4 (b) =====================
// 2017-06-08 Q5.pdf
// Both caches direct mapped, 16384 bytes each. Data block 8 bytes, instruction block 16 bytes.
// A loop of 5 instructions starts at 0x00401008 and runs 8 times. The only memory instruction is
//     MIPS: lb $t0,-4($t1)        RISC-V: lb t0,-4(t1)
// t1 = 0x60000006 before the loop and is increased by 1 right after the lb.
// Instruction cache miss rate? Data cache miss rate? Which locality?
// Key: instruction 5% (2/40), data 25% (2/8).
//      Instruction cache: both temporal and spatial. Data cache: spatial only.
//
// Unknown on the exam: WHERE in the loop the lb is. It does not matter, because
// the two caches are separate. Here it is put first.
void item4b(void) {
    printf("\nItem 4(b): 5-instruction loop from 0x00401008, 8 rounds, lb t0,-4(t1)\n");
    setupCache(ICACHE, 16384, 16, 1);
    setupCache(DCACHE, 16384, 8, 1);

    unsigned int t1 = 0x60000006;

    for (int round = 0; round < 8; round++) {
        unsigned int pc = 0x00401008;
        cacheAccess(ICACHE, pc, "lb");    pc = pc + 4;
        cacheAccess(DCACHE, t1 - 4, "lb");       // lb t0,-4(t1): reads ONE byte at t1 - 4
        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t1 = t1 + 1;                             // t1 is increased by one
        cacheAccess(ICACHE, pc, "instr"); pc = pc + 4;
        cacheAccess(ICACHE, pc, "instr"); pc = pc + 4;
        cacheAccess(ICACHE, pc, "instr"); pc = pc + 4;
    }

    checkPattern(ICACHE, "MHMHH");
    checkMissRate(ICACHE, 1, 20);                // 5% = 1/20
    checkPattern(DCACHE, "MHHHHHMH");
    checkMissRate(DCACHE, 1, 4);                 // 25% = 1/4
    checkLocality(ICACHE, BOTH);
    checkLocality(DCACHE, SPATIAL);
}

// ===================== Item 5 (b) =====================
// 2018-01-15 Q5.pdf
// 2-way instruction cache with LRU, 8192 bytes, 256 sets. State before execution:
//   Set 0x46, way 1: valid = 1, tag = 0x40012
//   Set 0x46, way 2: valid = 0, tag = 0xff012
//   Set 0x47, way 1: valid = 0, tag = 0x40012
//   Set 0x47, way 2: valid = 1, tag = 0x30010
// An instruction at 0x40012460 is executed. Hit or miss?
// Key: HIT. The address has tag 0x40012 and set 0x46; way 1 of set 0x46 is valid with that tag.

// Put the exam's cache state into the simulator (used by item 5 (b) and (c)).
void item5_loadState(void) {
    int capacity = 8192;
    int numberOfSets = 256;
    int numberOfWays = 2;
    int blockBytes = capacity / (numberOfSets * numberOfWays);   // 8192 / (256 x 2) = 16 bytes
    setupCache(ICACHE, capacity, blockBytes, numberOfWays);

    // Array index 0 is the exam's "way 1", index 1 is "way 2".
    valid[ICACHE][0x46][0] = 1;   tags[ICACHE][0x46][0] = 0x40012;
    valid[ICACHE][0x46][1] = 0;   tags[ICACHE][0x46][1] = 0xff012;
    valid[ICACHE][0x47][0] = 0;   tags[ICACHE][0x47][0] = 0x40012;
    valid[ICACHE][0x47][1] = 1;   tags[ICACHE][0x47][1] = 0x30010;
}

void item5b(void) {
    printf("\nItem 5(b): 2-way LRU cache with a given state, instruction at 0x40012460\n");
    item5_loadState();
    printf("  before:\n");
    printSet(ICACHE, 0x46);

    int hit = cacheAccess(ICACHE, 0x40012460, "instr");

    if (hit == 1) {
        printf("  answer: HIT    key: HIT   OK\n");
    } else {
        printf("  answer: MISS   key: HIT   WRONG\n");
    }
}

// ===================== Item 5 (c) =====================
// 2018-01-15 Q5.pdf
// Same starting state as (b). An instruction at 0x40012474 is executed.
// Does the cache state change? What changes?
// Key: Yes. It is a MISS: in set 0x47 way 1 is invalid, and way 2 is valid but has another tag.
//      The block goes into way 1: its valid bit becomes 1, its tag is written (it already
//      had the same value, 0x40012), and the block data is updated.
//
// Note: the tag in way 1 already matched, but valid = 0 means "this is rubbish",
// so it is still a miss. No LRU choice is needed because an empty way exists.
void item5c(void) {
    printf("\nItem 5(c): same state, instruction at 0x40012474. What changes?\n");
    item5_loadState();
    printf("  before:\n");
    printSet(ICACHE, 0x47);

    int hit = cacheAccess(ICACHE, 0x40012474, "instr");

    printf("  after:\n");
    printSet(ICACHE, 0x47);

    // What the key says must be true afterwards:
    int ok = 1;
    if (hit != 0) ok = 0;                                   // it is a miss
    if (valid[ICACHE][0x47][0] != 1) ok = 0;                // way 1 became valid
    if (tags[ICACHE][0x47][0] != 0x40012) ok = 0;           // way 1 tag is 0x40012 (same value as before)
    if (valid[ICACHE][0x47][1] != 1) ok = 0;                // way 2 is untouched
    if (tags[ICACHE][0x47][1] != 0x30010) ok = 0;
    printf("  answer: miss, set 0x47 way 1 gets valid = 1 (tag rewritten with the same value, block loaded)\n");
    if (ok == 1) {
        printf("  key: miss, valid bit of set 0x47 way 1 is set to 1   OK\n");
    } else {
        printf("  key: miss, valid bit of set 0x47 way 1 is set to 1   WRONG\n");
    }
}

// ===================== Item 6 (b) =====================
// 2018-03-15 Q5.pdf
// Direct mapped instruction cache, 2048 bytes, 256 sets.
// A loop of 5 instructions (the last one is the conditional jump) starts at
// 0x40001000 and runs 10 times. Hit rate? Temporal, spatial or both?
// Key: 47/50, both temporal and spatial
void item6b(void) {
    printf("\nItem 6(b): loop of 5 instructions from 0x40001000, 10 rounds\n");
    int capacity = 2048;
    int numberOfSets = 256;
    int blockBytes = capacity / (numberOfSets * 1);      // 2048 / 256 = 8 bytes = 2 instructions
    setupCache(ICACHE, capacity, blockBytes, 1);

    for (int round = 0; round < 10; round++) {
        show = (round < 2);
        unsigned int pc = 0x40001000;
        for (int i = 0; i < 5; i++) {
            cacheAccess(ICACHE, pc, "instr");
            pc = pc + 4;
        }
    }
    show = 1;
    printf("    ... rounds 3 to 10 are all hits, like round 2\n");

    checkHitRate(ICACHE, 47, 50);
    checkLocality(ICACHE, BOTH);
}

// ===================== Item 7 (b) =====================
// 2018-06-05 Q5.pdf
// Both caches direct mapped, 4096 bytes. Instruction block 8 bytes, data block 16 bytes.
// Both empty at start. lui is at 0x40000004.
//
//   MIPS in the exam            RISC-V form
//   lui $t0,0x1001              lui t0,0x10010     (both give t0 = 0x10010000)
//   lw  $t1,0x0($t0)            lw  t1,0x0(t0)
//   lw  $t1,0x8($t0)            lw  t1,0x8(t0)
//   lw  $t1,0x14($t0)           lw  t1,0x14(t0)
//
// i. Instruction cache miss rate?  ii. Data cache miss rate?  iii. Which locality?
// Key: i) 3/4, ii) 2/3, iii) only spatial locality (no address is used twice)
void item7b(void) {
    printf("\nItem 7(b): lui + three lw, lui at 0x40000004\n");
    setupCache(ICACHE, 4096, 8, 1);
    setupCache(DCACHE, 4096, 16, 1);

    unsigned int pc = 0x40000004;
    unsigned int t0 = 0x1001 << 16;              // MIPS lui: 0x1001 in the top 16 bits

    cacheAccess(ICACHE, pc, "lui");  pc = pc + 4;

    cacheAccess(ICACHE, pc, "lw");   pc = pc + 4;
    cacheAccess(DCACHE, t0 + 0x0, "lw");

    cacheAccess(ICACHE, pc, "lw");   pc = pc + 4;
    cacheAccess(DCACHE, t0 + 0x8, "lw");

    cacheAccess(ICACHE, pc, "lw");   pc = pc + 4;
    cacheAccess(DCACHE, t0 + 0x14, "lw");

    checkMissRate(ICACHE, 3, 4);
    checkMissRate(DCACHE, 2, 3);
    checkLocality(ICACHE, SPATIAL);
    checkLocality(DCACHE, SPATIAL);
}

// ===================== Item 8 (b) =====================
// 2019-01-12-riscv Q5.pdf
// Direct mapped caches, both empty at start.
// Instruction cache: 4096 bytes, 256 blocks. Data cache: 1024 bytes, block 8 bytes.
// lui is at 0x00040100.
//
//         lui  t0,0x3b2
//         ori  t0,t0,0x210
//         addi t1,zero,5
//         addi t2,zero,0
//         addi t3,zero,0
//   loop: lw   t2,4(t0)
//         add  t3,t3,t2
//         addi t0,t0,4
//         addi t1,t1,-1
//         bne  t1,zero,loop
//
// i. Data cache miss rate?  ii. Instruction cache miss rate?
// iii. Data cache locality?  iv. Instruction cache locality?
// Key: i) 60% (miss, miss, hit, miss, hit), ii) 10% (3/30),
//      iii) spatial only, iv) both temporal and spatial
void item8b(void) {
    printf("\nItem 8(b): loop with lw t2,4(t0), 5 rounds, lui at 0x00040100\n");
    int instructionBlock = 4096 / 256;           // capacity / blocks = 16 bytes = 4 instructions
    setupCache(ICACHE, 4096, instructionBlock, 1);
    setupCache(DCACHE, 1024, 8, 1);

    unsigned int pc = 0x00040100;
    unsigned int t0, t1;

    cacheAccess(ICACHE, pc, "lui");   pc = pc + 4;
    t0 = 0x3b2 << 12;                            // lui: 0x3b2 in the top 20 bits -> 0x003b2000
    cacheAccess(ICACHE, pc, "ori");   pc = pc + 4;
    t0 = t0 | 0x210;                             // -> 0x003b2210
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    t1 = 5;
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    unsigned int loop = pc;

    while (t1 != 0) {                            // bne t1,zero,loop
        pc = loop;
        cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
        cacheAccess(DCACHE, t0 + 4, "lw");       // lw t2,4(t0): reads address t0 + 4
        cacheAccess(ICACHE, pc, "add");   pc = pc + 4;
        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t0 = t0 + 4;                             // addi t0,t0,4
        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t1 = t1 - 1;                             // addi t1,t1,-1
        cacheAccess(ICACHE, pc, "bne");   pc = pc + 4;
    }

    checkPattern(DCACHE, "MMHMH");
    checkMissRate(DCACHE, 3, 5);                 // 60% = 3/5
    checkMissRate(ICACHE, 1, 10);                // 10% = 3/30
    checkLocality(DCACHE, SPATIAL);
    checkLocality(ICACHE, BOTH);
}

// ===================== Item 9 (b) =====================
// 2019-03-15-riscv Q5.pdf
// Instruction cache with 4096 sets and block size 16 bytes, empty at start.
// 7 instructions run in a row (no branches), the first at 0x4000301C.
// i. Hit or miss for each of the seven?  ii. Hit rate as a fraction?
// Key: i) miss, miss, hit, hit, hit, miss, hit   ii) 4/7
//
// The question does not say how many ways the cache has. The key notes that the
// answer is the same for any number of ways. Here it is direct mapped (1 way).
void item9b(void) {
    printf("\nItem 9(b): seven instructions in a row from 0x4000301C\n");
    int numberOfSets = 4096;
    int blockBytes = 16;
    int capacity = numberOfSets * blockBytes * 1;        // sets x block size x ways
    setupCache(ICACHE, capacity, blockBytes, 1);

    unsigned int pc = 0x4000301C;
    for (int i = 0; i < 7; i++) {
        cacheAccess(ICACHE, pc, "instr");
        pc = pc + 4;
    }
    // 0x...1C is the LAST word of its block, so the very next instruction is in a new block.
    checkPattern(ICACHE, "MMHHHMH");
    checkHitRate(ICACHE, 4, 7);
}

// ===================== Item 10 (b) =====================
// 2019-06-05-riscv Q5.pdf
// Direct mapped caches, both empty at start.
// Data cache: 2048 bytes, 256 blocks. Instruction cache: 1024 bytes, block 16 bytes.
// The first instruction is at 0x00030200.
//
//   addi t2,zero,0x210
//   lui  t3,0x33002
//   or   t3,t3,t2
//   lw   t4,4(t3)
//   lw   t4,8(t3)
//
// i. Data cache miss rate?  ii. Instruction cache miss rate?  iii. Instruction cache locality?
// Key: i) 100%, ii) 40% (2/5), iii) spatial only
void item10b(void) {
    printf("\nItem 10(b): addi, lui, or, lw 4(t3), lw 8(t3), first at 0x00030200\n");
    int dataBlock = 2048 / 256;                  // capacity / blocks = 8 bytes
    setupCache(DCACHE, 2048, dataBlock, 1);
    setupCache(ICACHE, 1024, 16, 1);

    unsigned int pc = 0x00030200;
    unsigned int t2, t3;

    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    t2 = 0x210;                                  // addi t2,zero,0x210
    cacheAccess(ICACHE, pc, "lui");   pc = pc + 4;
    t3 = 0x33002u << 12;                         // lui t3,0x33002 -> 0x33002000
    cacheAccess(ICACHE, pc, "or");    pc = pc + 4;
    t3 = t3 | t2;                                // -> 0x33002210

    cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
    cacheAccess(DCACHE, t3 + 4, "lw");           // 0x33002214: last word of block 0x33002210
    cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
    cacheAccess(DCACHE, t3 + 8, "lw");           // 0x33002218: first word of the NEXT block

    checkMissRate(DCACHE, 1, 1);                 // 100%
    checkMissRate(ICACHE, 2, 5);                 // 40%
    checkLocality(ICACHE, SPATIAL);
}

// ===================== Item 11 (b) =====================
// 2020-01-13-riscv Q5.pdf
// Six instructions run in a row. Number 1 and number 6 are lw, the others are add or sll.
// The first lw is at 0x40001004.
// Both caches direct mapped, 4096 bytes. Instruction block 8 bytes, data block 16 bytes.
// The first lw reads 0x80002000, the second lw reads 0x80002010. Both caches empty at start.
// i. Instruction cache miss rate?  ii. Data cache miss rate?  iii. Instruction locality?
// Key: i) 2/3 (miss, miss, hit, miss, hit, miss), ii) 100%, iii) spatial only
void item11b(void) {
    printf("\nItem 11(b): six instructions from 0x40001004, lw first and last\n");
    setupCache(ICACHE, 4096, 8, 1);
    setupCache(DCACHE, 4096, 16, 1);

    unsigned int pc = 0x40001004;

    cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
    cacheAccess(DCACHE, 0x80002000, "lw");       // address given in the question

    for (int i = 0; i < 4; i++) {                // instructions 2 to 5: add or sll, no memory access
        cacheAccess(ICACHE, pc, "add");
        pc = pc + 4;
    }

    cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
    cacheAccess(DCACHE, 0x80002010, "lw");       // 16 bytes later = exactly the next data block

    checkPattern(ICACHE, "MMHMHM");
    checkMissRate(ICACHE, 2, 3);
    checkMissRate(DCACHE, 1, 1);                 // 100%
    checkLocality(ICACHE, SPATIAL);
}

// ===================== Item 12 (b) =====================
// 2020-03-11-riscv Q5.pdf
// Both caches direct mapped, 4096 bytes. Instruction cache 256 sets, data cache 512 sets.
// Both empty at start. addi is at 0x40001004.
//
//   addi s0,0x750        (printed like this in the exam; the key reads it as s0 = 0x750,
//                         that is  addi s0,zero,0x750)
//   lw   t0,4(s0)
//   lw   t1,8(s0)
//
// (i) Instruction cache miss rate?  (ii) Data cache miss rate?
// Key: (i) 1/3, (ii) 100%
void item12b(void) {
    printf("\nItem 12(b): addi, lw 4(s0), lw 8(s0), addi at 0x40001004\n");
    int instructionBlock = 4096 / 256;           // capacity / sets = 16 bytes (direct mapped)
    int dataBlock = 4096 / 512;                  // 8 bytes
    setupCache(ICACHE, 4096, instructionBlock, 1);
    setupCache(DCACHE, 4096, dataBlock, 1);

    unsigned int pc = 0x40001004;
    unsigned int s0;

    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    s0 = 0x750;

    cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
    cacheAccess(DCACHE, s0 + 4, "lw");           // 0x754: last word of block 0x750
    cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
    cacheAccess(DCACHE, s0 + 8, "lw");           // 0x758: first word of the next block

    // All three instructions (0x...04, 08, 0c) sit in the block 0x40001000-0x4000100f.
    checkMissRate(ICACHE, 1, 3);
    checkMissRate(DCACHE, 1, 1);                 // 100%
}

// ===================== Item 13 (b) =====================
// 2020-06-05-riscv Q5.pdf
// Direct mapped caches, both empty at start.
// Instruction cache: 2048 bytes, 256 blocks. Data cache: 1024 bytes, block 16 bytes.
// lui is at 0x002c0110.
//
//         lui  t0,0xa0201
//         ori  t0,t0,0x720
//         addi t1,zero,6
//         addi t2,zero,0
//         addi t3,zero,0
//   loop: lw   t2,8(t0)
//         add  t3,t3,t2
//         addi t0,t0,4
//         addi t1,t1,-1
//         bne  t1,zero,loop
//
// i. Data cache miss rate?  ii. Data cache locality?  iii. Instruction cache locality?
// Key: i) 1/3 (miss, hit, miss, hit, hit, hit), ii) spatial only, iii) both temporal and spatial
// The instruction cache miss rate is not asked; the program prints it anyway.
void item13b(void) {
    printf("\nItem 13(b): loop with lw t2,8(t0), 6 rounds, lui at 0x002c0110\n");
    int instructionBlock = 2048 / 256;           // capacity / blocks = 8 bytes = 2 instructions
    setupCache(ICACHE, 2048, instructionBlock, 1);
    setupCache(DCACHE, 1024, 16, 1);

    unsigned int pc = 0x002c0110;
    unsigned int t0, t1;

    cacheAccess(ICACHE, pc, "lui");   pc = pc + 4;
    t0 = 0xa0201u << 12;                         // lui -> 0xa0201000
    cacheAccess(ICACHE, pc, "ori");   pc = pc + 4;
    t0 = t0 | 0x720;                             // -> 0xa0201720
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    t1 = 6;
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    unsigned int loop = pc;

    while (t1 != 0) {                            // bne t1,zero,loop
        pc = loop;
        cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
        cacheAccess(DCACHE, t0 + 8, "lw");       // lw t2,8(t0): reads address t0 + 8
        cacheAccess(ICACHE, pc, "add");   pc = pc + 4;
        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t0 = t0 + 4;                             // addi t0,t0,4
        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t1 = t1 - 1;                             // addi t1,t1,-1
        cacheAccess(ICACHE, pc, "bne");   pc = pc + 4;
    }

    checkPattern(DCACHE, "MHMHHH");
    checkMissRate(DCACHE, 1, 3);
    checkLocality(DCACHE, SPATIAL);
    checkLocality(ICACHE, BOTH);
    checkMissRate(ICACHE, 0, 0);                 // not asked: no key
}

// ===================== Extra Q5-Q7 (b) =====================
// Q5-Q7.png (not one of the 13 sittings)
// Both caches direct mapped with 1024 sets. Data block 8 bytes, instruction block 16 bytes.
// Both empty at start. The first addi is at 0x0040100c.
//
//   MIPS in the exam                RISC-V form
//         addi $t0,$zero,100              addi t0,zero,100
//         lui  $s0,0x1001                 lui  s0,0x10010
//         ori  $s0,$s0,400                ori  s0,s0,400
//   loop: lw   $t1,0($s0)           loop: lw   t1,0(s0)
//         add  $s0,$s0,8                  addi s0,s0,8     (a number needs addi)
//         addi $t0,$t0,-1                 addi t0,t0,-1
//         bne  $t0,$zero,loop             bne  t0,zero,loop
//
// Data cache miss rate? Instruction cache miss rate? Which locality for each?
// Key: data 100%, neither temporal nor spatial locality.
//      instruction 3/403, both temporal and spatial locality.
void extra_Q5_7b(void) {
    printf("\nExtra Q5-Q7(b): loop with lw t1,0(s0), s0 moves 8 bytes per round, 100 rounds\n");
    int numberOfSets = 1024;
    setupCache(DCACHE, numberOfSets * 8 * 1, 8, 1);      // capacity = sets x block size x ways
    setupCache(ICACHE, numberOfSets * 16 * 1, 16, 1);

    unsigned int pc = 0x0040100c;
    unsigned int t0, s0;

    cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
    t0 = 100;
    cacheAccess(ICACHE, pc, "lui");   pc = pc + 4;
    s0 = 0x1001 << 16;                           // MIPS lui -> 0x10010000
    cacheAccess(ICACHE, pc, "ori");   pc = pc + 4;
    s0 = s0 | 400;                               // 400 = 0x190 -> 0x10010190
    unsigned int loop = pc;

    int round = 0;
    while (t0 != 0) {                            // bne t0,zero,loop
        show = (round < 2);                      // print only the first two rounds
        pc = loop;
        cacheAccess(ICACHE, pc, "lw");    pc = pc + 4;
        cacheAccess(DCACHE, s0 + 0, "lw");       // lw t1,0(s0)
        cacheAccess(ICACHE, pc, "add");   pc = pc + 4;
        s0 = s0 + 8;                             // a whole data block further: never a neighbour
        cacheAccess(ICACHE, pc, "addi");  pc = pc + 4;
        t0 = t0 - 1;
        cacheAccess(ICACHE, pc, "bne");   pc = pc + 4;
        round++;
    }
    show = 1;
    printf("    ... rounds 3 to 100 look like round 2\n");

    checkMissRate(DCACHE, 1, 1);                 // 100%
    checkMissRate(ICACHE, 3, 403);
    checkLocality(DCACHE, NEITHER);
    checkLocality(ICACHE, BOTH);
}

// ===================== Extra Q5-Q8 (b) =====================
// Q5-Q8.png (not one of the 13 sittings)
// Both caches direct mapped, block 16 bytes, 1024 bytes each, empty at start.
// The first instruction is at 0x04010004.
//
//   MIPS in the exam            RISC-V form
//   lui $t1,0x1001              lui t1,0x10010     (both give t1 = 0x10010000)
//   lw  $t0,0x8($t1)            lw  t0,0x8(t1)
//   lw  $t0,0x4($t1)            lw  t0,0x4(t1)
//   lw  $t0,0x20($t1)           lw  t0,0x20(t1)
//   lw  $t0,0x4($t1)            lw  t0,0x4(t1)
//
// For each line: instruction cache hit or miss, data cache hit, miss or no access?
// Key: line 1: I miss, no data access
//      line 2: I hit,  D miss
//      line 3: I hit,  D hit
//      line 4: I miss, D miss
//      line 5: I hit,  D hit
void extra_Q5_8b(void) {
    printf("\nExtra Q5-Q8(b): lui + four lw, first instruction at 0x04010004\n");
    setupCache(ICACHE, 1024, 16, 1);
    setupCache(DCACHE, 1024, 16, 1);

    unsigned int pc = 0x04010004;
    unsigned int t1 = 0x1001 << 16;              // MIPS lui -> 0x10010000
    int offset[4] = { 0x8, 0x4, 0x20, 0x4 };     // the four lw offsets, in order

    cacheAccess(ICACHE, pc, "lui");  pc = pc + 4;
    for (int i = 0; i < 4; i++) {
        cacheAccess(ICACHE, pc, "lw");
        pc = pc + 4;
        cacheAccess(DCACHE, t1 + offset[i], "lw");
    }
    // The last lw is a hit: 0x10010020 went to set 2, so it did not push out set 0.
    checkPattern(ICACHE, "MHHMH");
    checkPattern(DCACHE, "MHMH");
}

// ===================== Extra Q5-Q11 (b), first bullet =====================
// Q5-Q11.png (not one of the 13 sittings; older exam, "Module 4")
// Direct mapped instruction cache, 256 sets, block 16 bytes, empty at start.
// A program of 256 instructions in a row (no branches) starts at 0x00001000. Hit rate?
// Key: 75% (a miss at every fourth instruction)
// (The second bullet, the size of the valid bit and tag, is in module5_cache_fields.c.)
void extra_Q5_11b(void) {
    printf("\nExtra Q5-Q11(b): 256 instructions in a row from 0x00001000\n");
    int numberOfSets = 256;
    setupCache(ICACHE, numberOfSets * 16 * 1, 16, 1);

    unsigned int pc = 0x00001000;
    for (int i = 0; i < 256; i++) {
        show = (i < 8);                          // print only the first 8
        cacheAccess(ICACHE, pc, "instr");
        pc = pc + 4;
    }
    show = 1;
    printf("    ... the pattern miss, hit, hit, hit repeats\n");

    checkPattern(ICACHE, "MHHHMHHH");
    checkHitRate(ICACHE, 3, 4);                  // 75%
}

// ===================== Extra Q5-Q12 (b), statement 3 =====================
// Q5-Q12.png (not one of the 13 sittings)
// True or false: "Direct mapped cache, block 16 bytes, 256 blocks. A load byte reads
// 0xff20215e and then another load byte reads 0x10005150. The second load is a miss."
// Key: True. Both use the same set (0x15) but have different tags.
// (Statements 1, 2 and 4 are in module5_theory.md.)
void extra_Q5_12b3(void) {
    printf("\nExtra Q5-Q12(b) statement 3: lb from 0xff20215e, then lb from 0x10005150\n");
    int blocks = 256;
    int blockBytes = 16;
    setupCache(DCACHE, blocks * blockBytes, blockBytes, 1);   // capacity = blocks x block size

    unsigned int first = 0xff20215e;
    unsigned int second = 0x10005150;

    // The set number of each address, worked out by hand:
    unsigned int firstSet = (first / blockBytes) % blocks;    // direct mapped: sets = blocks
    unsigned int secondSet = (second / blockBytes) % blocks;
    printf("  set of first = 0x%02x, set of second = 0x%02x   key = 0x15 for both   ", firstSet, secondSet);
    if (firstSet == 0x15 && secondSet == 0x15) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }

    cacheAccess(DCACHE, first, "lb");
    int hit = cacheAccess(DCACHE, second, "lb");  // same set, other tag: the first block is thrown out

    if (hit == 0) {
        printf("  second load is a miss -> statement is TRUE    key = TRUE   OK\n");
    } else {
        printf("  second load is a hit -> statement is FALSE    key = TRUE   WRONG\n");
    }
}

// ===================== Extra Q5-Q13 (b) =====================
// Q5-Q13.png (not one of the 13 sittings)
// Direct mapped data cache, 256 blocks, block size 4 words (16 bytes), empty at start.
//
//   MIPS in the exam                RISC-V form
//         li   $t4,0x02232130             li   t4,0x02232130
//         li   $t5,0x03f4a134             li   t5,0x03f4a134
//         addi $t0,$0,75                  addi t0,zero,75
//   loop: beq  $t0,$0,done          loop: beq  t0,zero,done
//         lw   $t1,0($t4)                 lw   t1,0(t4)
//         lw   $t2,4($t5)                 lw   t2,4(t5)
//         addi $t0,$t0,-1                 addi t0,t0,-1
//         j    loop                       j    loop
//   done:
//
// Data cache miss rate?
// Key: 100%. Both loads use set 0x13 with different tags, so they throw each other out
//      every time (a "conflict").
//
// Unknown on the exam: the addresses of the instructions. Only the data cache is asked.
void extra_Q5_13b(void) {
    printf("\nExtra Q5-Q13(b): two lw that fight over the same set, 75 rounds\n");
    int blocks = 256;
    int blockBytes = 4 * 4;                      // 4 words x 4 bytes
    setupCache(DCACHE, blocks * blockBytes, blockBytes, 1);

    unsigned int t4 = 0x02232130;
    unsigned int t5 = 0x03f4a134;
    int t0 = 75;

    int round = 0;
    while (t0 != 0) {                            // beq t0,zero,done
        show = (round < 2);                      // print only the first two rounds
        cacheAccess(DCACHE, t4 + 0, "lw");       // lw t1,0(t4) -> 0x02232130
        cacheAccess(DCACHE, t5 + 4, "lw");       // lw t2,4(t5) -> 0x03f4a138
        t0 = t0 - 1;                             // addi t0,t0,-1
        round++;
    }
    show = 1;
    printf("    ... rounds 3 to 75 look like round 2\n");

    checkMissRate(DCACHE, 1, 1);                 // 100%
    // Note: both addresses ARE used again and again (temporal locality in the program),
    // but a direct mapped cache cannot keep both, so it gets no hits from it.
}

int main() {
    item1a();
    item2b();
    item3c();
    item4b();
    item5b();
    item5c();
    item6b();
    item7b();
    item8b();
    item9b();
    item10b();
    item11b();
    item12b();
    item13b();
    extra_Q5_7b();
    extra_Q5_8b();
    extra_Q5_11b();
    extra_Q5_12b3();
    extra_Q5_13b();
}
