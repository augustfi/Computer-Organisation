#include <stdio.h>

// =====================================================================
// Module 3 (exam question 3): LOGIC DESIGN, part 3: the REGISTER FILE
//
// A register file is a small memory made of registers.
//   - address width  = how many bits select a register.
//                      n address bits -> 2^n registers.
//   - data width     = how many bits ONE register holds (what one port reads
//                      or writes in parallel).
//   - capacity       = number of registers x data width.
//   - The NUMBER of read and write ports does not change the capacity.
//     Every port has the same address width and the same data width.
//
// Item numbers = exam sittings in date order (see module3_combinational.c).
//
// Run:  gcc -Wall module3_register_file.c -o regfile && ./regfile
// =====================================================================


// ---------- Building blocks used by the register-file circuit ----------

int AND(int a, int b) { return a & b; }

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


// ---------- Helpers for the size calculations ----------

// 2 to the power n. Shifting 1 left n times doubles it n times.
int powerOfTwo(int n) {
    return 1 << n;
}

// The opposite: how many address bits are needed for "count" registers?
int addressBits(int count) {
    int bits = 0;
    while ((1 << bits) < count) {
        bits++;
    }
    return bits;
}

// Capacity in bytes from address width and data width (both in bits).
int capacityBytes(int addrBits, int dataBits) {
    int registers = powerOfTwo(addrBits);
    int bytesEach = dataBits / 8;        // 8 bits = 1 byte
    printf("  2^%d = %d registers, each %d bits = %d bytes\n", addrBits, registers, dataBits, bytesEach);
    return registers * bytesEach;
}

// Compare one number with the key.
void check(char* what, int got, int key, char* unit) {
    printf("  %s = %d %s   key = %d %s   ", what, got, unit, key, unit);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 2 (b) =====================
// Source: 2017-01-12 Q3.pdf
// Question: a register file with one read port (A1, RD1) and one write port
//   (A2, WD2, WE2). Address signals are 10 bits, data signals are 16 bits.
//   How many bytes can it store?
// Key: 2048 bytes
void item2b(void) {
    printf("\nItem 2(b): 10-bit address, 16-bit data. Capacity?\n");
    check("capacity", capacityBytes(10, 16), 2048, "bytes");
}

// ===================== Item 3 (c) =====================
// Source: 2017-03-16 Q3.pdf
// Question: one read port, one write port. The address signal is 10 bits.
//   16 bits can be read in parallel in one clock cycle. Capacity in bytes?
// Key: 2048 bytes
void item3c(void) {
    printf("\nItem 3(c): 10-bit address, 16 bits read in parallel. Capacity?\n");
    check("capacity", capacityBytes(10, 16), 2048, "bytes");
}

// ===================== Item 4 (a) =====================
// Source: 2017-06-08 Q3.pdf
// Question: one read port, one write port. Data signals are 32 bits,
//   address signals are 8 bits. How many bytes can it store?
// Key: 1024 bytes
void item4a(void) {
    printf("\nItem 4(a): 8-bit address, 32-bit data. Capacity?\n");
    check("capacity", capacityBytes(8, 32), 1024, "bytes");
}

// ===================== Item 5 (a) =====================
// Source: 2018-01-15 Q3.pdf
// Question: two read ports, one write port. Data signals are 16 bits.
//   The register file stores 1024 bytes in total. How wide are the address signals?
// Key: 9 bits
void item5a(void) {
    printf("\nItem 5(a): 1024 bytes, 16-bit data. Address width?\n");
    int bytesEach = 16 / 8;                  // one register = 2 bytes
    int registers = 1024 / bytesEach;        // 512 registers
    printf("  %d bytes / %d bytes each = %d registers\n", 1024, bytesEach, registers);
    check("address width", addressBits(registers), 9, "bits");
}

// ===================== Item 7 (b) =====================
// Source: 2018-06-05 Q3.pdf
// Question: 3 read ports, 1 write port. The read addresses are 12 bits wide.
//   8 bits can be read from each read port. Capacity in bytes? And how wide is
//   the address of the write port, if every position can be written?
// Key: 4096 bytes, and 12 bits
void item7b(void) {
    printf("\nItem 7(b): 12-bit read address, 8-bit data. Capacity and write address width?\n");
    int capacity = capacityBytes(12, 8);
    check("capacity", capacity, 4096, "bytes");
    // The write port must reach the same registers as the read ports,
    // so it needs enough address bits for all of them.
    int registers = powerOfTwo(12);
    check("write address width", addressBits(registers), 12, "bits");
}

// ===================== Item 10 (c) =====================
// Source: 2019-06-05-riscv Q3.pdf
// Question: three read ports, one write port. Address signals are 6 bits.
//   The register file stores 2048 BITS in total. How many bits can be written
//   in parallel with the write port?
// Key: 32 bits
void item10c(void) {
    printf("\nItem 10(c): 6-bit address, 2048 bits in total. Write port width?\n");
    int registers = powerOfTwo(6);           // 64 registers
    int bitsEach = 2048 / registers;         // bits in one register
    printf("  2^6 = %d registers, 2048 bits / %d = %d bits each\n", registers, registers, bitsEach);
    check("write port width", bitsEach, 32, "bits");
}

// ===================== Item 11 (b) =====================
// Source: 2020-01-13-riscv Q3.pdf
// Question: one read port and one 16-bit write port. Address signals are 6 bits.
//   Capacity in bytes?
// Key: 128 bytes
void item11b(void) {
    printf("\nItem 11(b): 6-bit address, 16-bit data. Capacity?\n");
    check("capacity", capacityBytes(6, 16), 128, "bytes");
}

// ===================== Item 12 (b) =====================
// Source: 2020-03-11-riscv Q3.pdf
// Question: 3 read ports, 1 write port. The capacity is 128 bytes.
//   Address signals are 4 bits. How many bits wide is the write port?
// Key: 64 bits
void item12b(void) {
    printf("\nItem 12(b): 128 bytes, 4-bit address. Write port width?\n");
    int registers = powerOfTwo(4);           // 16 registers
    int bytesEach = 128 / registers;         // 8 bytes in one register
    int bitsEach = bytesEach * 8;
    printf("  2^4 = %d registers, 128 bytes / %d = %d bytes each\n", registers, registers, bytesEach);
    check("write port width", bitsEach, 64, "bits");
}

// ===================== Item 13 (b) =====================
// Source: 2020-06-05-riscv Q3.pdf
// Question: two read ports and two write ports. Data signals are 32 bits,
//   address signals are 10 bits. How many bytes can it store?
// Key: 4096 bytes   (the number of ports does not matter)
void item13b(void) {
    printf("\nItem 13(b): 10-bit address, 32-bit data, 2+2 ports. Capacity?\n");
    check("capacity", capacityBytes(10, 32), 4096, "bytes");
}


// =====================================================================
// The register-file CIRCUIT (items 8(a) and 13(c) use the same figure)
//
// The circuit in words:
//   - A 2:4 decoder with the 2-bit write address A3 as input (so N = 2).
//   - Four AND gates. AND gate i has decoder output i and WE3 as inputs.
//     Its output is the enable (EN) of register i.
//     So register i is enabled only if A3 = i AND WE3 = 1.
//   - Four registers, each M bits wide. All have WD3 as data input and CLK as
//     clock. A register loads WD3 on a rising clock edge, but only if EN = 1.
//   - Two 4:1 muxes read the registers: select A1 gives RD1, select A2 gives RD2.
//     Mux input 00 = register 0, 01 = register 1, 10 = register 2, 11 = register 3.
//   - The marked signal B = the output of AND gate 2 (decoder output 10 AND WE3).
//   - The marked signal A = the output of register 3 (the wire to mux input 11).
//
// A value that the question does not let us know is stored as UNKNOWN.
// =====================================================================

#define UNKNOWN -1

int reg[4];          // what the four registers hold

// Compute the four enable signals from A3 and WE3.
void enables(int a3, int we3, int en[4]) {
    int dec[4];
    decoder(a3, dec);
    for (int i = 0; i < 4; i++) {
        en[i] = AND(dec[i], we3);
    }
}

// A rising clock edge: every register with EN = 1 loads WD3.
void risingEdge(int a3, int we3, int wd3) {
    int en[4];
    enables(a3, we3, en);
    for (int i = 0; i < 4; i++) {
        if (en[i] == 1) {
            reg[i] = wd3;
        }
    }
}

// Read port 2: a 4:1 mux with A2 as select.
int readRD2(int a2) {
    return mux4(reg[0], reg[1], reg[2], reg[3], a2);
}

// Print a value, or the word unknown.
void printValue(int v) {
    if (v == UNKNOWN) {
        printf("unknown");
    } else {
        printf("%d", v);
    }
}

// Compare a value (may be UNKNOWN) with the key (may be UNKNOWN).
void checkValue(char* what, int got, int key) {
    printf("  %s = ", what);
    printValue(got);
    printf("   key = ");
    printValue(key);
    if (got == key) {
        printf("   OK\n");
    } else {
        printf("   WRONG\n");
    }
}

// Question (iv) in both items: all registers hold "old". WD3, A2, A3 are given,
// but WE3 is NOT given. So we try both WE3 = 0 and WE3 = 1. If RD2 is the same
// in both cases it is a known number, otherwise it is unknown.
int rd2AfterEdge(int old, int wd3, int a2, int a3) {
    int result[2];
    for (int we3 = 0; we3 <= 1; we3++) {
        for (int i = 0; i < 4; i++) {
            reg[i] = old;                // all registers hold the old value
        }
        risingEdge(a3, we3, wd3);        // CLK goes from 0 to 1
        result[we3] = readRD2(a2);
        printf("  if WE3 = %d: registers = %d %d %d %d, RD2 = register %d = %d\n",
               we3, reg[0], reg[1], reg[2], reg[3], a2, result[we3]);
    }
    if (result[0] == result[1]) {
        return result[0];
    }
    return UNKNOWN;
}

// ===================== Item 8 (a) =====================
// Source: 2019-01-12-riscv Q3.pdf (also Q3.6.png)
// Question (circuit described above):
//   (i)   the register file stores 256 bits in total. What is M?
//   (ii)  A3 = 3, WE3 = 1, WD3 = 7, CLK = 0 at one point in time. What is A?
//   (iii) A3 = 3, WE3 = 1. What is B?
//   (iv)  WD3 = 9, CLK = 0, A2 = 2, A3 = 3, all registers hold 7.
//         What is RD2 after the clock switched to 1?
// Key: (i) 64 bits   (ii) unknown   (iii) 0   (iv) 7
void item8a(void) {
    printf("\nItem 8(a): the register-file circuit\n");

    // (i) four registers share the 256 bits
    int m = 256 / 4;
    check("(i) M", m, 64, "bits");

    // (ii) We are not told what the registers hold, so they start as UNKNOWN.
    // CLK is 0: there is no rising edge, so nothing is loaded, even though
    // register 3 is enabled. A is the output of register 3.
    for (int i = 0; i < 4; i++) {
        reg[i] = UNKNOWN;
    }
    int a = reg[3];
    checkValue("(ii) A", a, UNKNOWN);

    // (iii) B = AND(decoder output 10, WE3). A3 = 3 makes output 11 high, not 10.
    int en[4];
    enables(3, 1, en);
    checkValue("(iii) B", en[2], 0);

    // (iv) register 3 may be written, but RD2 reads register 2.
    int rd2 = rd2AfterEdge(7, 9, 2, 3);
    checkValue("(iv) RD2", rd2, 7);
}

// ===================== Item 13 (c) =====================
// Source: 2020-06-05-riscv Q3.pdf (also Q3x2.png)
// Question (same circuit, described above):
//   (i)   the register file stores 128 bits in total. What is M?
//   (ii)  A3 = 2, WE3 = 0, WD3 = 3, CLK = 1 at one point in time. What is A?
//   (iii) A3 = 2, WE3 = 1. What is B?
//   (iv)  WD3 = 40, CLK = 0, A2 = 3, A3 = 1, all registers hold 20.
//         What is RD2 after the clock switched to 1?
// Key: (i) 32 bits   (ii) unknown   (iii) 1   (iv) 20
void item13c(void) {
    printf("\nItem 13(c): the register-file circuit\n");

    // (i) four registers share the 128 bits
    int m = 128 / 4;
    check("(i) M", m, 32, "bits");

    // (ii) The registers start as UNKNOWN. Even if CLK = 1 came from a rising
    // edge, WE3 = 0 means that no register is enabled, so nothing is loaded.
    for (int i = 0; i < 4; i++) {
        reg[i] = UNKNOWN;
    }
    risingEdge(2, 0, 3);
    int a = reg[3];
    checkValue("(ii) A", a, UNKNOWN);

    // (iii) B = AND(decoder output 10, WE3). A3 = 2 makes output 10 high.
    int en[4];
    enables(2, 1, en);
    checkValue("(iii) B", en[2], 1);

    // (iv) register 1 may be written, but RD2 reads register 3.
    int rd2 = rd2AfterEdge(20, 40, 3, 1);
    checkValue("(iv) RD2", rd2, 20);
}


int main() {
    item2b();
    item3c();
    item4a();
    item5a();
    item7b();
    item8a();
    item10c();
    item11b();
    item12b();
    item13b();
    item13c();
}
