#include <stdio.h>

// =====================================================================
// Module 5: MEMORY HIERARCHY, part 1: the GEOMETRY of a cache
//
// Every exam item is its own function (item1b, item2a, ...).
// Each item starts from the numbers the question gives, works out the
// answer one step per line, and compares it with the exam's answer ("key").
//
// Run:  gcc -Wall module5_cache_fields.c -o fields && ./fields
//
// ---------------------------------------------------------------------
// The few rules that every item uses
//
// A cache is a table. One row is called a SET. A set holds N BLOCKS,
// one per WAY. (Direct mapped = 1 way. "N-way" = N blocks in every set.)
//
//   blocks      = capacity / block size
//   sets        = blocks / ways        = capacity / (block size x ways)
//   valid bits  = blocks               (every block has its own valid bit)
//
// The address is cut in three fields:
//
//   |      tag      |    set    | byte offset |
//
//   byte offset bits = log2(block size)   (which byte inside the block)
//   set bits         = log2(sets)         (which row)
//   tag bits         = address bits - set bits - byte offset bits
//
// On a miss the WHOLE block is loaded, so  bytes loaded = block size,
// and  instructions loaded = block size / 4  (one instruction = 4 bytes).
// =====================================================================


// ---------- Helpers ----------

// log2 for a power of two: how many times can n be halved until it is 1?
// Example: log2int(8) = 3 because 8 -> 4 -> 2 -> 1 is three steps.
int log2int(int n) {
    int bits = 0;
    while (n > 1) {
        n = n / 2;
        bits++;
    }
    return bits;
}

// 2 to the power of bits. Example: power2(7) = 128.
int power2(int bits) {
    return 1 << bits;
}

// Print our answer next to the exam's answer.
void check(char* what, int got, int key) {
    printf("  %-22s = %-6d key = %-6d ", what, got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// The same, but printed as hexadecimal numbers.
void checkHex(char* what, unsigned int got, unsigned int key) {
    printf("  %-22s = 0x%-6x key = 0x%-6x ", what, got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 1 (b) =====================
// 2016-06-09 Q5.pdf
// 32-bit processor, 4-way cache, block size 8 bytes, tag field 19 bits.
// What is the capacity in bytes?
// Key: 32768 bytes
void item1b(void) {
    printf("\nItem 1(b): 4-way, block 8 bytes, tag 19 bits -> capacity?\n");
    int addressBits = 32;
    int ways = 4;
    int blockSize = 8;
    int tagBits = 19;

    int offsetBits = log2int(blockSize);                 // 2^3 = 8  -> 3 bits
    int setBits = addressBits - tagBits - offsetBits;    // what is left: 32 - 19 - 3 = 10
    int sets = power2(setBits);                          // 2^10 = 1024 sets
    int blocks = sets * ways;                            // 4 blocks in every set
    int capacity = blocks * blockSize;                   // every block is 8 bytes

    printf("  offset bits = %d, set bits = %d, sets = %d, blocks = %d\n", offsetBits, setBits, sets, blocks);
    check("capacity (bytes)", capacity, 32768);
}

// ===================== Item 2 (a) =====================
// 2017-01-12 Q5.pdf
// 32-bit processor, 2-way cache, 512 blocks, capacity 4096 bytes.
// How many bits are the tag, set and byte offset fields?
// Key: tag 21, set 8, byte offset 3
void item2a(void) {
    printf("\nItem 2(a): 2-way, 512 blocks, 4096 bytes -> tag, set, offset bits?\n");
    int addressBits = 32;
    int ways = 2;
    int blocks = 512;
    int capacity = 4096;

    int blockSize = capacity / blocks;                   // 4096 / 512 = 8 bytes
    int offsetBits = log2int(blockSize);                 // 3
    int sets = blocks / ways;                            // 512 / 2 = 256
    int setBits = log2int(sets);                         // 8
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 8 - 3

    printf("  block size = %d, sets = %d\n", blockSize, sets);
    check("tag bits", tagBits, 21);
    check("set bits", setBits, 8);
    check("byte offset bits", offsetBits, 3);
}

// ===================== Item 3 (a) i =====================
// 2017-03-16 Q5.pdf
// Direct mapped data cache, capacity 1024 bytes, 128 sets. A load byte misses.
// i. How many bytes are loaded from main memory because of the miss?
// Key: 8 bytes (a whole block is always loaded, even for a load BYTE)
// (Part ii, "which three parts are updated", is in module5_theory.md.)
void item3a(void) {
    printf("\nItem 3(a) i: direct mapped, 1024 bytes, 128 sets -> bytes loaded on a miss?\n");
    int capacity = 1024;
    int sets = 128;
    int ways = 1;                                        // direct mapped = 1 way

    int blocks = sets * ways;                            // 128 blocks
    int blockSize = capacity / blocks;                   // 1024 / 128 = 8
    int bytesLoaded = blockSize;                         // a miss loads one whole block

    check("bytes loaded", bytesLoaded, 8);
}

// ===================== Item 3 (b) =====================
// 2017-03-16 Q5.pdf
// N-way cache, capacity 4096 bytes, set field 7 bits, tag field 21 bits, 32-bit processor.
// Block size? Number of valid bits? Associativity N?
// Key: block size 16 bytes, 256 valid bits, 2-way
void item3b(void) {
    printf("\nItem 3(b): 4096 bytes, set 7 bits, tag 21 bits -> block size, valid bits, N?\n");
    int addressBits = 32;
    int capacity = 4096;
    int setBits = 7;
    int tagBits = 21;

    int offsetBits = addressBits - setBits - tagBits;    // 32 - 7 - 21 = 4
    int blockSize = power2(offsetBits);                  // 2^4 = 16 bytes
    int blocks = capacity / blockSize;                   // 4096 / 16 = 256
    int validBits = blocks;                              // one valid bit per block
    int sets = power2(setBits);                          // 2^7 = 128
    int ways = blocks / sets;                            // 256 / 128 = 2

    check("block size (bytes)", blockSize, 16);
    check("valid bits", validBits, 256);
    check("associativity N", ways, 2);
}

// ===================== Item 4 (a) =====================
// 2017-06-08 Q5.pdf
// 4-way cache, block size 32 bytes, capacity 4096 bytes, 16-bit processor.
// Sizes of the tag field and the set field?
// Key: tag 6 bits, set 5 bits
void item4a(void) {
    printf("\nItem 4(a): 4-way, block 32, 4096 bytes, 16-bit addresses -> tag and set bits?\n");
    int addressBits = 16;                                // NOTE: 16-bit processor, not 32
    int ways = 4;
    int blockSize = 32;
    int capacity = 4096;

    int offsetBits = log2int(blockSize);                 // 2^5 = 32 -> 5
    int sets = capacity / (blockSize * ways);            // 4096 / (32 x 4) = 32
    int setBits = log2int(sets);                         // 5
    int tagBits = addressBits - setBits - offsetBits;    // 16 - 5 - 5

    printf("  offset bits = %d, sets = %d\n", offsetBits, sets);
    check("tag bits", tagBits, 6);
    check("set bits", setBits, 5);
}

// ===================== Item 5 (a) =====================
// 2018-01-15 Q5.pdf
// 32-bit processor, direct mapped data cache, block size 32 bytes, tag 20 bits.
// Number of sets? Capacity in bytes?
// Key: 128 sets, 4096 bytes
void item5a(void) {
    printf("\nItem 5(a): direct mapped, block 32, tag 20 bits -> sets and capacity?\n");
    int addressBits = 32;
    int ways = 1;
    int blockSize = 32;
    int tagBits = 20;

    int offsetBits = log2int(blockSize);                 // 5
    int setBits = addressBits - tagBits - offsetBits;    // 32 - 20 - 5 = 7
    int sets = power2(setBits);                          // 128
    int capacity = sets * ways * blockSize;              // 128 x 1 x 32

    check("sets", sets, 128);
    check("capacity (bytes)", capacity, 4096);
}

// ===================== Item 6 (a) =====================
// 2018-03-15 Q5.pdf
// 32-bit processor, 4-way data cache, block size 8 bytes, capacity 16384 bytes.
// Tag bits? Number of valid bits?
// Key: tag 20 bits, 2048 valid bits
void item6a(void) {
    printf("\nItem 6(a): 4-way, block 8, 16384 bytes -> tag bits and valid bits?\n");
    int addressBits = 32;
    int ways = 4;
    int blockSize = 8;
    int capacity = 16384;

    int offsetBits = log2int(blockSize);                 // 3
    int sets = capacity / (blockSize * ways);            // 16384 / 32 = 512
    int setBits = log2int(sets);                         // 9
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 9 - 3
    int blocks = capacity / blockSize;                   // 16384 / 8 = 2048
    int validBits = blocks;                              // one per block

    printf("  sets = %d, set bits = %d\n", sets, setBits);
    check("tag bits", tagBits, 20);
    check("valid bits", validBits, 2048);
}

// ===================== Item 7 (a) =====================
// 2018-06-05 Q5.pdf
// Data cache, 32-bit processor, capacity 16384 bytes, tag 20 bits, block 16 bytes.
// Number of sets? Associativity? Number of valid bits?
// Key: 256 sets, associativity 4, 1024 valid bits
void item7a(void) {
    printf("\nItem 7(a): 16384 bytes, tag 20 bits, block 16 -> sets, associativity, valid bits?\n");
    int addressBits = 32;
    int capacity = 16384;
    int tagBits = 20;
    int blockSize = 16;

    int offsetBits = log2int(blockSize);                 // 4
    int setBits = addressBits - tagBits - offsetBits;    // 32 - 20 - 4 = 8
    int sets = power2(setBits);                          // 256
    int blocks = capacity / blockSize;                   // 16384 / 16 = 1024
    int ways = blocks / sets;                            // 1024 / 256 = 4
    int validBits = blocks;

    check("sets", sets, 256);
    check("associativity", ways, 4);
    check("valid bits", validBits, 1024);
}

// ===================== Item 8 (a) =====================
// 2019-01-12-riscv Q5.pdf
// 32-bit RISC-V, 4-way cache, capacity 4096 bytes, block size 8 bytes.
// i. Number of valid bits?   ii. Tag field size?
// Key: i) 512 valid bits, ii) 22 bits
void item8a(void) {
    printf("\nItem 8(a): 4-way, 4096 bytes, block 8 -> valid bits and tag bits?\n");
    int addressBits = 32;
    int ways = 4;
    int capacity = 4096;
    int blockSize = 8;

    int blocks = capacity / blockSize;                   // 512
    int validBits = blocks;
    int sets = blocks / ways;                            // 512 / 4 = 128
    int setBits = log2int(sets);                         // 7
    int offsetBits = log2int(blockSize);                 // 3
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 7 - 3

    check("valid bits", validBits, 512);
    check("tag bits", tagBits, 22);
}

// ===================== Item 9 (a) =====================
// 2019-03-15-riscv Q5.pdf
// Direct mapped data cache, 16-bit processor, block size 8 bytes.
// Set number 31 has valid bit 1 and the 6-bit tag 100101. All other sets are invalid.
// i. Number of sets?   ii. Capacity?   iii. An address where a load word hits (hex)?
// Key: i) 128 sets, ii) 1024 bytes, iii) 0x94f8
void item9a(void) {
    printf("\nItem 9(a): 16-bit, direct mapped, block 8, tag 6 bits -> sets, capacity, a hit address?\n");
    int addressBits = 16;
    int ways = 1;
    int blockSize = 8;
    int tagBits = 6;
    unsigned int tag = 0x25;                             // binary 100101
    unsigned int set = 31;                               // the only valid set

    int offsetBits = log2int(blockSize);                 // 3
    int setBits = addressBits - tagBits - offsetBits;    // 16 - 6 - 3 = 7
    int sets = power2(setBits);                          // 128
    int capacity = sets * ways * blockSize;              // 128 x 8

    check("sets", sets, 128);
    check("capacity (bytes)", capacity, 1024);

    // To hit, the address must have this tag and this set number.
    // Build it by putting the fields in their places:   tag | set | byte offset
    unsigned int byteOffset = 0;                         // any offset in the block hits; the key picks 0
    unsigned int address = (tag << (setBits + offsetBits)) | (set << offsetBits) | byteOffset;
    checkHex("hit address", address, 0x94f8);

    // A word is 4 bytes and starts on an address that is a multiple of 4,
    // so the other word in this block (byte offset 4) is also a correct answer.
    printf("  also correct: 0x%x (byte offset 4, same block)\n", address | 4);
}

// ===================== Item 10 (a) =====================
// 2019-06-05-riscv Q5.pdf
// 2-way cache, 32-bit RISC-V, capacity 2048 bytes, 128 sets.
// i. Block size?   ii. Valid bits?   iii. Tag size?
// Key: i) 8 bytes, ii) 256 bits, iii) 22 bits
void item10a(void) {
    printf("\nItem 10(a): 2-way, 2048 bytes, 128 sets -> block size, valid bits, tag bits?\n");
    int addressBits = 32;
    int ways = 2;
    int capacity = 2048;
    int sets = 128;

    int blocks = sets * ways;                            // 128 x 2 = 256
    int blockSize = capacity / blocks;                   // 2048 / 256 = 8
    int validBits = blocks;
    int offsetBits = log2int(blockSize);                 // 3
    int setBits = log2int(sets);                         // 7
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 7 - 3

    check("block size (bytes)", blockSize, 8);
    check("valid bits", validBits, 256);
    check("tag bits", tagBits, 22);
}

// ===================== Item 11 (a) =====================
// 2020-01-13-riscv Q5.pdf
// 32-bit RISC-V, instruction cache, capacity 1024 bytes, tag 23 bits, block 8 bytes.
// Byte offset field size? Set field size? Associativity? Valid bits?
// Key: 3 bits, 6 bits, 2-way, 128 valid bits
void item11a(void) {
    printf("\nItem 11(a): 1024 bytes, tag 23 bits, block 8 -> offset bits, set bits, ways, valid bits?\n");
    int addressBits = 32;
    int capacity = 1024;
    int tagBits = 23;
    int blockSize = 8;

    int offsetBits = log2int(blockSize);                 // 3
    int setBits = addressBits - tagBits - offsetBits;    // 32 - 23 - 3 = 6
    int sets = power2(setBits);                          // 64
    int blocks = capacity / blockSize;                   // 1024 / 8 = 128
    int ways = blocks / sets;                            // 128 / 64 = 2
    int validBits = blocks;

    check("byte offset bits", offsetBits, 3);
    check("set bits", setBits, 6);
    check("associativity", ways, 2);
    check("valid bits", validBits, 128);
}

// ===================== Item 12 (a) =====================
// 2020-03-11-riscv Q5.pdf
// 4-way instruction cache, capacity 2048 bytes, 32-bit RISC-V, 256 blocks in total.
// i. Instructions fetched on a miss?  ii. Sets?  iii. Tag size?  iv. Valid bits?
// Key: i) 2 instructions, ii) 64 sets, iii) 23 bits, iv) 256 valid bits
void item12a(void) {
    printf("\nItem 12(a): 4-way, 2048 bytes, 256 blocks -> instructions per miss, sets, tag, valid bits?\n");
    int addressBits = 32;
    int ways = 4;
    int capacity = 2048;
    int blocks = 256;

    int blockSize = capacity / blocks;                   // 2048 / 256 = 8 bytes
    int instructionsPerMiss = blockSize / 4;             // one RISC-V instruction is 4 bytes
    int sets = blocks / ways;                            // 256 / 4 = 64
    int setBits = log2int(sets);                         // 6
    int offsetBits = log2int(blockSize);                 // 3
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 6 - 3
    int validBits = blocks;

    check("instructions per miss", instructionsPerMiss, 2);
    check("sets", sets, 64);
    check("tag bits", tagBits, 23);
    check("valid bits", validBits, 256);
}

// ===================== Item 13 (a) =====================
// 2020-06-05-riscv Q5.pdf
// 4-way cache, 32-bit addresses, capacity 4096 bytes, block size 16 bytes.
// Tag, set and byte offset bits? Number of valid bits?
// Key: tag 22, set 6, byte offset 4, 256 valid bits
void item13a(void) {
    printf("\nItem 13(a): 4-way, 4096 bytes, block 16 -> tag, set, offset bits, valid bits?\n");
    int addressBits = 32;
    int ways = 4;
    int capacity = 4096;
    int blockSize = 16;

    int offsetBits = log2int(blockSize);                 // 4
    int sets = capacity / (blockSize * ways);            // 4096 / 64 = 64
    int setBits = log2int(sets);                         // 6
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 6 - 4
    int validBits = sets * ways;                         // one per block = 64 x 4

    check("tag bits", tagBits, 22);
    check("set bits", setBits, 6);
    check("byte offset bits", offsetBits, 4);
    check("valid bits", validBits, 256);
}

// ===================== Extra Q5-Q7 (a) =====================
// Q5-Q7.png (not one of the 13 sittings)
// 4-way cache, 16-bit addresses, capacity 2048 bytes, block size 8 bytes.
// Tag, set and byte offset bits? Number of valid bits?
// Key: tag 7, set 6, byte offset 3, 256 valid bits
void extra_Q5_7a(void) {
    printf("\nExtra Q5-Q7(a): 4-way, 16-bit addresses, 2048 bytes, block 8\n");
    int addressBits = 16;
    int ways = 4;
    int capacity = 2048;
    int blockSize = 8;

    int offsetBits = log2int(blockSize);                 // 3
    int sets = capacity / (blockSize * ways);            // 2048 / 32 = 64
    int setBits = log2int(sets);                         // 6
    int tagBits = addressBits - setBits - offsetBits;    // 16 - 6 - 3
    int validBits = sets * ways;                         // 64 x 4

    check("tag bits", tagBits, 7);
    check("set bits", setBits, 6);
    check("byte offset bits", offsetBits, 3);
    check("valid bits", validBits, 256);
}

// ===================== Extra Q5-Q8 (a) =====================
// Q5-Q8.png (not one of the 13 sittings)
// 32-bit processor, 4-way cache, capacity 8192 bytes, set field 7 bits.
// Block size? Tag bits? Valid bits?
// Key: block size 16 bytes, tag 21 bits, 512 valid bits
void extra_Q5_8a(void) {
    printf("\nExtra Q5-Q8(a): 4-way, 8192 bytes, set field 7 bits\n");
    int addressBits = 32;
    int ways = 4;
    int capacity = 8192;
    int setBits = 7;

    int sets = power2(setBits);                          // 128
    int blocks = sets * ways;                            // 128 x 4 = 512
    int blockSize = capacity / blocks;                   // 8192 / 512 = 16
    int offsetBits = log2int(blockSize);                 // 4
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 7 - 4
    int validBits = blocks;

    check("block size (bytes)", blockSize, 16);
    check("tag bits", tagBits, 21);
    check("valid bits", validBits, 512);
}

// ===================== Extra Q5-Q10 (a) =====================
// Q5-Q10.png (not one of the 13 sittings; an older exam that calls this "Module 4")
// Direct mapped data cache, capacity 4096 bytes, block size 16 bytes.
//
//   MIPS in the exam                  RISC-V form
//   lui  $t1,0x1000                   lui  t1,0x10000
//   addi $t1,$t1,0x0864               addi t1,t1,0x864   (see note)
//   sw   $t0,4($t1)                   sw   t0,4(t1)
//   lb   $t0,16($t1)                  lb   t0,16(t1)
//
// Note: in RISC-V the addi number is 12 bits WITH a sign, so 0x864 does not fit.
// Real RISC-V code would be  lui t1,0x10001  then  addi t1,t1,-1948.
// The register value, 0x10000864, is the same, and only that matters here.
//
// Which sets (rows) do sw and lb use? Answer in hex.
// Key: sw uses set 0x86, lb uses set 0x87
void extra_Q5_10a(void) {
    printf("\nExtra Q5-Q10(a): which set do sw 4(t1) and lb 16(t1) use?\n");
    int capacity = 4096;
    int blockSize = 16;
    int ways = 1;

    int sets = capacity / (blockSize * ways);            // 4096 / 16 = 256
    int offsetBits = log2int(blockSize);                 // 4

    unsigned int t1 = (0x1000 << 16) + 0x0864;           // MIPS lui puts 0x1000 in the top 16 bits
    unsigned int swAddress = t1 + 4;                     // sw t0,4(t1)
    unsigned int lbAddress = t1 + 16;                    // lb t0,16(t1)
    printf("  t1 = 0x%08x, sw address = 0x%08x, lb address = 0x%08x\n", t1, swAddress, lbAddress);

    // Set number: throw away the byte offset bits, then keep the set bits.
    // "% sets" keeps the lowest log2(sets) bits (same as & (sets - 1)).
    unsigned int swSet = (swAddress >> offsetBits) % sets;
    unsigned int lbSet = (lbAddress >> offsetBits) % sets;

    checkHex("set for sw", swSet, 0x86);
    checkHex("set for lb", lbSet, 0x87);
}

// ===================== Extra Q5-Q10 (b) =====================
// Q5-Q10.png
// 32-bit processor, 4-way cache, capacity 4096 bytes, block 8 bytes.
// Tag, set and byte offset bits?
// Key: tag 22, set 7, byte offset 3
void extra_Q5_10b(void) {
    printf("\nExtra Q5-Q10(b): 4-way, 4096 bytes, block 8\n");
    int addressBits = 32;
    int ways = 4;
    int capacity = 4096;
    int blockSize = 8;

    int offsetBits = log2int(blockSize);                 // 3
    int sets = capacity / (blockSize * ways);            // 4096 / 32 = 128
    int setBits = log2int(sets);                         // 7
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 7 - 3

    check("tag bits", tagBits, 22);
    check("set bits", setBits, 7);
    check("byte offset bits", offsetBits, 3);
}

// ===================== Extra Q5-Q11 (a) =====================
// Q5-Q11.png (not one of the 13 sittings; older exam, "Module 4")
// Data cache, 1024 bytes of data, block 4 bytes, set field 6 bits.
// What is N for this N-way cache?
// Key: N = 4
void extra_Q5_11a(void) {
    printf("\nExtra Q5-Q11(a): 1024 bytes, block 4, set field 6 bits -> N?\n");
    int capacity = 1024;
    int blockSize = 4;
    int setBits = 6;

    int sets = power2(setBits);                          // 64
    int blocks = capacity / blockSize;                   // 1024 / 4 = 256
    int ways = blocks / sets;                            // 256 / 64

    check("associativity N", ways, 4);
}

// ===================== Extra Q5-Q11 (b), second bullet =====================
// Q5-Q11.png
// Direct mapped instruction cache, 32-bit processor, 256 sets, block 16 bytes (4096 bytes of data).
// What else is stored in each set besides the data, and how many bits is each?
// Key: a valid bit (1 bit) and the tag (32 - 8 - 4 = 20 bits)
// (The first bullet, the hit rate, is in module5_cache_sim.c.)
void extra_Q5_11b_sizes(void) {
    printf("\nExtra Q5-Q11(b): 256 sets, block 16 -> size of the valid bit and the tag?\n");
    int addressBits = 32;
    int sets = 256;
    int blockSize = 16;

    int setBits = log2int(sets);                         // 8
    int offsetBits = log2int(blockSize);                 // 4
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 8 - 4
    int validBitsPerSet = 1;                             // direct mapped: one block per set, one valid bit

    check("valid bit (bits)", validBitsPerSet, 1);
    check("tag (bits)", tagBits, 20);
}

// ===================== Extra Q5-Q12 (a) =====================
// Q5-Q12.png (not one of the 13 sittings)
// 2-way cache, 32-bit addresses, 1024 blocks in total, block size 16 bytes.
// Tag, set and byte offset bits? Number of valid bits?
// Key: 512 sets -> set 9 bits, byte offset 4 bits, tag 19 bits, 1024 valid bits
void extra_Q5_12a(void) {
    printf("\nExtra Q5-Q12(a): 2-way, 1024 blocks, block 16\n");
    int addressBits = 32;
    int ways = 2;
    int blocks = 1024;
    int blockSize = 16;

    int sets = blocks / ways;                            // 1024 / 2 = 512
    int setBits = log2int(sets);                         // 9
    int offsetBits = log2int(blockSize);                 // 4
    int tagBits = addressBits - setBits - offsetBits;    // 32 - 9 - 4
    int validBits = blocks;

    check("tag bits", tagBits, 19);
    check("set bits", setBits, 9);
    check("byte offset bits", offsetBits, 4);
    check("valid bits", validBits, 1024);
}

// ===================== Extra Q5-Q13 (a) =====================
// Q5-Q13.png (not one of the 13 sittings)
// N-way cache, word size 32 bits, capacity 4096 bytes, block size 8 WORDS, 32 sets.
// What is the associativity?
// Key: N = 4
void extra_Q5_13a(void) {
    printf("\nExtra Q5-Q13(a): 4096 bytes, block 8 words, 32 sets -> N?\n");
    int capacity = 4096;
    int wordSize = 4;                                    // a 32-bit word is 4 bytes
    int blockWords = 8;
    int sets = 32;

    int blockSize = blockWords * wordSize;               // 8 words = 32 bytes
    int blocks = capacity / blockSize;                   // 4096 / 32 = 128
    int ways = blocks / sets;                            // 128 / 32

    printf("  block size = %d bytes, blocks = %d\n", blockSize, blocks);
    check("associativity N", ways, 4);
}

// ===================== Extra Q5-Q13 (c), statement 3 =====================
// Q5-Q13.png
// True or false: "Virtual memory with page size 1024 bytes, 32-bit virtual address,
// 28-bit physical address. A page table translates 20-bit virtual page numbers
// into 16-bit physical page numbers."
// Key: False. 10 bits are the page offset, so it is 22-bit virtual page numbers
//      and 18-bit physical page numbers.
//
// This works just like the byte offset of a cache:
//   page offset bits = log2(page size)
//   page number bits = address bits - page offset bits
void extra_Q5_13c3(void) {
    printf("\nExtra Q5-Q13(c) statement 3: page size 1024, 32-bit virtual, 28-bit physical\n");
    int pageSize = 1024;
    int virtualBits = 32;
    int physicalBits = 28;

    int pageOffsetBits = log2int(pageSize);              // 2^10 = 1024 -> 10
    int virtualPageBits = virtualBits - pageOffsetBits;  // 32 - 10
    int physicalPageBits = physicalBits - pageOffsetBits; // 28 - 10

    check("virtual page bits", virtualPageBits, 22);
    check("physical page bits", physicalPageBits, 18);

    // The statement claims 20 and 16.
    if (virtualPageBits == 20 && physicalPageBits == 16) {
        printf("  statement is TRUE      key = FALSE   WRONG\n");
    } else {
        printf("  statement is FALSE     key = FALSE   OK\n");
    }
}

int main() {
    item1b();
    item2a();
    item3a();
    item3b();
    item4a();
    item5a();
    item6a();
    item7a();
    item8a();
    item9a();
    item10a();
    item11a();
    item12a();
    item13a();
    extra_Q5_7a();
    extra_Q5_8a();
    extra_Q5_10a();
    extra_Q5_10b();
    extra_Q5_11a();
    extra_Q5_11b_sizes();
    extra_Q5_12a();
    extra_Q5_13a();
    extra_Q5_13c3();
}
