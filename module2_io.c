#include <stdio.h>

// =====================================================================
// Module 2, Q2: I/O SYSTEMS - the C side of MEMORY-MAPPED I/O
//
// Every exam item is its own function (item4a, item7a, ...).
//
// Memory-mapped I/O in three lines:
//   - A "port" (LEDs, switches, buttons) looks like one word of memory
//     at a fixed address. Reading the word reads the switches, writing
//     the word turns LEDs on and off. Each bit is one LED / switch.
//   - In C you reach it with a pointer that holds the address:
//         volatile int* leds = (volatile int*) 0xabab0040;
//   - volatile tells the compiler: this value can change by itself
//     (somebody flips a switch), so really read it / write it every time.
//
// The addresses in the exam (0xabab0040 ...) do NOT exist on a normal
// computer. Reading them would crash the program. So in this file an
// ordinary variable plays the role of each port, and the pointer points
// at that variable instead. The exam's real line is in a comment beside it:
//
//         volatile int* leds = &leds_port;     // exam: (volatile int*) 0xabab0040
//
// ON THE EXAM, WRITE THE LINE FROM THE COMMENT.
//
// The four bit tricks every item uses:
//   x & mask      keep only the bits that are 1 in mask (the rest become 0)
//   x & ~mask     clear the bits that are 1 in mask (the rest stay)
//   x | bits      turn on the bits that are 1 in bits (the rest stay)
//   x >> n, x << n  move the bits n places right / left
//
// Run:  gcc -Wall module2_io.c -o io && ./io
// =====================================================================


// ---------- Helpers ----------

// Compare our number with the exam's answer. Printed in hex.
void check(int got, int key) {
    printf("  answer = 0x%x   key = 0x%x   ", got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Read bit number n of value (n = 0 is the lowest bit). Gives 0 or 1.
int getBit(int value, int n) {
    return (value >> n) & 1;
}


// ===================== Item 2 (b) =====================
// 2017-01-12 Q2.pdf   (same as Q2.12translate.png)
// The exam gives this C function and asks for it in assembly.
// The assembly is in module2_io.asm (item2b). Here the C itself is run,
// so you can see what number the assembly must return.
//
//     int get_status(){
//       volatile int* toggle = (volatile int*) 0xabab0020;
//       return (*toggle & 0x4) >> 2;
//     }
//
// What it does: 0x4 is binary 100, so & 0x4 keeps only bit 2.
// >> 2 moves that bit down to bit 0. The result is 0 or 1.
int toggle_port2;                            // stands in for the port at 0xabab0020

int get_status(void) {
    volatile int* toggle = &toggle_port2;    // exam: (volatile int*) 0xabab0020;
    return (*toggle & 0x4) >> 2;
}

void item2b(void) {
    printf("\nItem 2(b): get_status() returns bit 2 of the port\n");

    toggle_port2 = 0x1234;                   // binary ...0011 0100, bit 2 is 1
    int got = get_status();
    printf("  port = 0x%x\n", toggle_port2);
    check(got, getBit(toggle_port2, 2));     // compare with bit 2 read the slow way

    toggle_port2 = 0xfffb;                   // binary ...1111 1011, bit 2 is 0
    got = get_status();
    printf("  port = 0x%x\n", toggle_port2);
    check(got, getBit(toggle_port2, 2));
}


// ===================== Item 4 (a) =====================
// 2017-06-08 Q2.pdf   (same as Q2.10filltranslate.png)
//
//     volatile int* switches = (volatile int*) 0xabab0020;
//     volatile int* leds = (volatile int*) 0xabab0040;
//     while(/* MISSING C CODE EXPRESSION */){
//       *leds = (*switches >> 4) & 1;
//     }
//
// Bit 0 is switch number 1, bit 7 is switch number 8. 1 = on.
// Write the missing expression so that the loop exits when switch 8 is off.
// Key: *switches & 0x80
// (Part (b), the assembly, is in module2_io.asm, item4b.)
int switches_port4;                          // stands in for the port at 0xabab0020
int leds_port4;                              // stands in for the port at 0xabab0040

void item4a(void) {
    printf("\nItem 4(a): the missing while condition\n");

    // Step 1: switch number 8 is bit 7 (switch 1 is bit 0, so subtract 1).
    int bit = 8 - 1;
    // Step 2: a mask with only that bit set.
    int mask = 1 << bit;                     // 1000 0000 = 0x80
    printf("  switch 8 is bit %d, mask = 1 << %d:\n", bit, bit);
    check(mask, 0x80);
    printf("  missing expression:  *switches & 0x%x\n", mask);
    // The loop keeps going while the result is not 0 (switch 8 on)
    // and exits when it is 0 (switch 8 off).

    // Step 3: run the loop.
    // Nobody can flip a real switch here, so a list of values plays the
    // role of the user. Each round the next value is put in the port.
    int user[] = {0x90, 0x80, 0xff, 0x7f};   // the last one has bit 7 = 0
    int round = 0;
    switches_port4 = user[0];

    volatile int* switches = &switches_port4;    // exam: (volatile int*) 0xabab0020;
    volatile int* leds = &leds_port4;            // exam: (volatile int*) 0xabab0040;
    while (*switches & 0x80) {                   // <-- the missing expression
        *leds = (*switches >> 4) & 1;            // LED 1 shows switch number 5 (bit 4)

        // ---------- ADDED (not in the exam): print, then the user moves the switches ----------
        printf("  switches = 0x%02x  switch 8 on   -> leds = %d\n", *switches, *leds);
        round++;
        switches_port4 = user[round];
        // ------------------------------------------------------------------------------------------
    }
    printf("  switches = 0x%02x  switch 8 off  -> loop exits after %d rounds\n", *switches, round);
}


// ===================== Item 7 (a) =====================
// 2018-06-05 Q2.pdf   (same as Q2.7filltranslate.png)
//
//     volatile int* switches = (volatile int*) 0x1b430010;
//     volatile int* leds = (volatile int*) 0xabbb8050;
//     int temp = ((*switches) >> 6) & 0x3;
//     *leds = (*leds & 0xff9f) | XXX;
//
// Some bits are read from the 16-bit switch port and used to turn LEDs
// on or off without changing the other LEDs. What is XXX?
// Key: temp << 5
// (Part (b), the assembly, is in module2_io.asm, item7b.)
int switches_port7;                          // stands in for the port at 0x1b430010
int leds_port7;                              // stands in for the port at 0xabbb8050

void item7a(void) {
    printf("\nItem 7(a): the missing expression XXX\n");

    // How to find it: the mask 0xff9f CLEARS some LED bits. Those are the
    // bits that are 0 in the mask. The new bits must go exactly there.
    //     0xff9f = 1111 1111 1001 1111
    //                         ^^        bits 6 and 5 are 0
    // temp is 2 bits wide (& 0x3) and sits at bits 1..0. To move it to
    // bits 6..5 it must be shifted left by 5.
    int mask = 0xff9f;
    int shift = 0;
    while (getBit(mask, shift) == 1) {       // look for the lowest 0 bit in the mask
        shift++;
    }
    printf("  lowest cleared bit in 0x%x is bit %d, so XXX = temp << %d\n", mask, shift, shift);
    printf("  shift amount: ");
    check(shift, 5);

    // Now run the code with the answer filled in.
    switches_port7 = 0x0080;                 // bits 7..6 = 10
    leds_port7 = 0xffff;                     // all 16 LEDs on

    volatile int* switches = &switches_port7;    // exam: (volatile int*) 0x1b430010;
    volatile int* leds = &leds_port7;            // exam: (volatile int*) 0xabbb8050;
    int temp = ((*switches) >> 6) & 0x3;         // temp = switch bits 7..6, moved down to bits 1..0
    printf("  switches = 0x%04x -> temp = %d, leds before = 0x%04x\n", *switches, temp, *leds);
    *leds = (*leds & 0xff9f) | (temp << 5);      // <-- XXX is temp << 5

    // Expected: LED bits 6..5 are now 10, every other LED is still on:
    //     1111 1111 1101 1111 = 0xffdf
    printf("  leds after:\n");
    check(*leds, 0xffdf);
}


// ===================== Item 10 (a) i, iii, iv =====================
// 2019-06-05-riscv Q2.pdf   (same as Q2.3code.png and Q2.4translate.png)
//
//     int getvals(){
//       int *pushbutton = (int*) 0x10010040;
//       int *switches = (int*) 0x10010050;
//       int v=0;
//       while(*pushbutton & 1){
//         v = (*switches >> 4) & 0xf;
//       }
//       return v;
//     }
//
// Button 1 is bit 0, button 2 is bit 1 (1 = pushed).
// Switch 1 is bit 4 ... switch 8 is bit 11 (1 = up).
//   i.   The code is missing a keyword. Which one?
//        Key: volatile (the pointers should be declared with volatile).
//   iii. What makes the function exit?
//        Key: push button number 1 is released (not pushed).
//   iv.  What value is returned?
//        Key: the 4-bit value formed by switches 4, 3, 2 and 1, as they
//        were when button 1 was released.
// (Part ii, the assembly, is in module2_io.asm, item10a.)
//
// Why volatile (i): without it the compiler may think "nothing in this
// loop changes *pushbutton, so I only need to read it once". Then the
// loop would never notice that the button was released. volatile forces
// a real read from the port every time round the loop.
// (You cannot see this difference by running this file: gcc without -O
//  does not optimise, and the port here is only a variable.)
int pushbutton_port10;                       // stands in for the port at 0x10010040
int switches_port10;                         // stands in for the port at 0x10010050

int getvals(void) {
    volatile int *pushbutton = &pushbutton_port10;   // exam: (volatile int*) 0x10010040;
    volatile int *switches = &switches_port10;       // exam: (volatile int*) 0x10010050;
    int v = 0;

    // ---------- ADDED (not in the exam) ----------
    int rounds = 0;
    // -----------------------------------------------

    while (*pushbutton & 1) {                // true as long as button 1 (bit 0) is pushed
        v = (*switches >> 4) & 0xf;          // switches 1..4 are bits 4..7: move down, keep 4 bits

        // ---------- ADDED (not in the exam): after 3 rounds the user lets go of button 1 ----------
        rounds++;
        printf("  round %d: button 1 pushed, v = %d\n", rounds, v);
        if (rounds == 3) {
            pushbutton_port10 = 0x2;         // bit 0 = 0: button 1 released (button 2 still pushed)
        }
        // -----------------------------------------------------------------------------------------------
    }

    return v;
}

void item10a(void) {
    printf("\nItem 10(a) i:   missing keyword: volatile   (key: volatile)\n");
    printf("Item 10(a) iii: the loop exits when button 1 is released\n");

    pushbutton_port10 = 0x3;                 // both buttons pushed
    switches_port10 = 0xa50;                 // 1010 0101 0000: switches 1 and 3 up (bits 4 and 6)
    int v = getvals();
    printf("  button 1 released -> the function returned\n");

    // (iv) The key says: the value formed by switches 4, 3, 2, 1.
    // Build that the slow way, one switch at a time, and compare.
    int s1 = getBit(switches_port10, 4);
    int s2 = getBit(switches_port10, 5);
    int s3 = getBit(switches_port10, 6);
    int s4 = getBit(switches_port10, 7);
    int expected = s4 * 8 + s3 * 4 + s2 * 2 + s1;
    printf("Item 10(a) iv:  returned value (switches 4,3,2,1 = %d%d%d%d):\n", s4, s3, s2, s1);
    check(v, expected);
}


// ===================== Item 13 (b) =====================
// 2020-06-05-riscv Q2.pdf   (same as Q2.6code.png and Q2.1translate.png)
// The exam gives these three C lines and asks for them in assembly.
// The assembly is in module2_io.asm (item13b). Here the C itself is run
// with the same port values, so you can see what the assembly must store.
//
//     volatile int* leds = (volatile int*) 0x1dbb8050;
//     volatile int* switches = (volatile int*) 0x22430030;
//     *leds = (*leds & 0x79f) | ((((*switches) >> 4) & 0x3) << 3);
int leds_port13;                             // stands in for the port at 0x1dbb8050
int switches_port13;                         // stands in for the port at 0x22430030

void item13b(void) {
    printf("\nItem 13(b): the three C lines, run step by step\n");

    leds_port13 = 0xfe7;                     // 1111 1110 0111
    switches_port13 = 0x20;                  // bits 5..4 = 10

    // First by hand, one small step at a time (this is also the order of
    // the assembly instructions):
    int a = switches_port13 >> 4;            // switch bits 5..4 moved down to bits 1..0
    int b = a & 0x3;                         // keep only those 2 bits      -> 2
    int c = b << 3;                          // move them up to bits 4..3   -> 0x10
    int d = leds_port13 & 0x79f;             // 0x79f = 0111 1001 1111: clears bits 6, 5 and bit 11 -> 0x787
    int expected = d | c;                    // put the two new bits in     -> 0x797
    printf("  switch bits = %d, shifted = 0x%x, leds masked = 0x%x\n", b, c, d);

    // Then the exam's own lines.
    volatile int* leds = &leds_port13;           // exam: (volatile int*) 0x1dbb8050;
    volatile int* switches = &switches_port13;   // exam: (volatile int*) 0x22430030;
    *leds = (*leds & 0x79f) | ((((*switches) >> 4) & 0x3) << 3);

    printf("  leds after (0x797 is also what module2_io.asm item13b must print):\n");
    check(*leds, expected);
}


// ===================== extra_Q2_14 =====================
// Q2.14createC.png   (older exam, not one of the 13 sittings)
// A 16-bit microcontroller runs at 1 MHz and has an 8-bit timer.
//   - 16-bit control register at 0xffa0. Bits 7..6 set the prescale:
//       11 = 1:8,  10 = 1:4,  01 = 1:2,  00 = no prescale.
//     No other bit of the control register may change.
//   - Period register at 0xffa2 (only the lowest 8 bits are used).
//   - sizeof(int) is 2 on that machine.
// Write the C code that sets the timer up for a period of 1 millisecond.
// Key:
//     volatile int* control = (volatile int*) 0xffa0;
//     volatile int* period  = (volatile int*) 0xffa2;
//     *control = (*control & ~0xc0) | (2 << 6);
//     *period  = 250;
//   (The key also accepts prescale 1:8 with period 125.)
int control_port14;                          // stands in for the register at 0xffa0
int period_port14;                           // stands in for the register at 0xffa2

void extra_Q2_14(void) {
    printf("\nextra_Q2_14: set up an 8-bit timer at 1 MHz for a 1 ms period\n");

    // ----- Part 1: the numbers (same kind of sum as in module2_timer.c) -----
    int frequency = 1000000;
    int ticks = frequency / 1000;            // clock ticks in 1 ms = 1000
    int max = 255;                           // biggest value in 8 bits
    printf("  ticks in 1 ms = %d, biggest 8-bit value = %d\n", ticks, max);

    // Try the prescales from small to big. The 2-bit code for each one
    // comes from the question: 00, 01, 10, 11.
    int prescale[] = {1, 2, 4, 8};
    int code[]     = {0, 1, 2, 3};
    int chosenCode = -1;
    int chosenPeriod = -1;
    for (int i = 0; i < 4; i++) {
        int p = ticks / prescale[i];
        printf("  prescale 1:%d (code %d) -> period = %4d   %s\n",
               prescale[i], code[i], p, p <= max ? "fits" : "too big");
        if (p <= max && chosenCode == -1) {  // take the first one that fits
            chosenCode = code[i];
            chosenPeriod = p;
        }
    }
    printf("  prescale code:  ");
    check(chosenCode, 2);
    printf("  period value:   ");
    check(chosenPeriod, 250);

    // ----- Part 2: the C code -----
    control_port14 = 0x1234;                 // some old value, to show the other bits survive
    period_port14 = 0;
    int before = control_port14;

    volatile int* control = &control_port14;     // exam: (volatile int*) 0xffa0;
    volatile int* period = &period_port14;       // exam: (volatile int*) 0xffa2;
    // 0xc0 = 1100 0000 = bits 7 and 6.  ~0xc0 = every bit EXCEPT 7 and 6.
    //   *control & ~0xc0   clears bits 7..6 and keeps the rest
    //   | (2 << 6)         puts the code 10 into bits 7..6
    *control = (*control & ~0xc0) | (2 << 6);
    *period = 250;

    // Check 1: bits 7..6 now hold the code 2.
    printf("  control: 0x%04x -> 0x%04x, bits 7..6:  ", before, *control);
    check((*control >> 6) & 0x3, 2);
    // Check 2: all the other bits are the same as before.
    printf("  other control bits unchanged:           ");
    check(*control & ~0xc0, before & ~0xc0);
    printf("  period register = %d\n", *period);
}


// ===================== extra_Q2_17 =====================
// Q2.17createC.png   (older exam, there called Module 5)
// Write a C program that, in an infinite loop:
//   - reads a 32-bit input port at address 0x800. Each bit is a push
//     button (1 = pushed). Bit 0 is button 0 ... bit 31 is button 31.
//   - writes an output port at address 0x700. Bits 0..5 are 6 LEDs (1 = on).
//   - if button number 3 is pushed, all 6 LEDs are on, otherwise all off.
// It must work even with an optimising compiler.
// Key:
//     volatile int* push_buttons = (volatile int*) 0x800;
//     volatile int* LED = (volatile int*) 0x700;
//     while(1){
//       if((*push_buttons) & 8)
//         *LED = 0x3f;
//       else
//         *LED = 0x0;
//     }
//   "volatile must be used to make sure that the compiler does not
//    optimize the memory mapped I/O accesses."
//   (The key also accepts & 4, for those who counted button 3 as the
//    third button. Counting from 0 as the question says, it is & 8.)
int buttons_port17;                          // stands in for the port at 0x800
int led_port17;                              // stands in for the port at 0x700

void extra_Q2_17(void) {
    printf("\nextra_Q2_17: button 3 pushed -> all 6 LEDs on\n");

    // The two constants, worked out:
    int buttonMask = 1 << 3;                 // button 3 is bit 3: 1000 = 8
    int allLeds = (1 << 6) - 1;              // six 1-bits: 100 0000 - 1 = 11 1111 = 0x3f
    printf("  mask for button 3:  ");
    check(buttonMask, 8);
    printf("  value for 6 LEDs:   ");
    check(allLeds, 0x3f);

    volatile int* push_buttons = &buttons_port17;    // exam: (volatile int*) 0x800;
    volatile int* LED = &led_port17;                 // exam: (volatile int*) 0x700;

    // The exam's loop is  while(1){ ... }  and never ends. Here it runs
    // once for each test value instead, so that the program can finish.
    int user[] = {0x0, 0x8, 0xfffffff7, 0xf, 0x4};
    for (int i = 0; i < 5; i++) {            // exam: while(1){
        buttons_port17 = user[i];            // ADDED (not in the exam): the user pushes buttons

        if ((*push_buttons) & 8)
            *LED = 0x3f;
        else
            *LED = 0x0;

        // ---------- ADDED (not in the exam): check this round ----------
        int expected = 0;
        if (getBit(buttons_port17, 3) == 1) {
            expected = allLeds;
        }
        printf("  buttons = 0x%08x:", buttons_port17);
        check(*LED, expected);
        // -----------------------------------------------------------------
    }
}


// ===================== extra_Q2_19 =====================
// Q2.19createC.png   (older exam)
// Draw a 7-segment display and number the segments 0 to 6. Show the C
// code that displays the number '1'. The display is mapped to address
// 0xff34. Bit value 1 = the segment is lit.
// Key (with the segment numbers from the lecture slides):
//     volatile int* segm = (volatile int*) 0xff34;
//     *segm = 6;
//
// The numbering from the lecture (also the figure in the 2019-03-15 exam):
//
//        0
//      -----
//   5 |     | 1
//     |  6  |
//      -----
//   4 |     | 2
//     |     |
//      -----
//        3
//
// Segment n is bit n of the port.
int segm_port19;                             // stands in for the port at 0xff34

// Draw the display for a port value. A segment is drawn if its bit is 1.
void draw7(int value) {
    printf("     %s\n", getBit(value, 0) ? " --- " : "     ");
    printf("     %c   %c\n", getBit(value, 5) ? '|' : ' ', getBit(value, 1) ? '|' : ' ');
    printf("     %s\n", getBit(value, 6) ? " --- " : "     ");
    printf("     %c   %c\n", getBit(value, 4) ? '|' : ' ', getBit(value, 2) ? '|' : ' ');
    printf("     %s\n", getBit(value, 3) ? " --- " : "     ");
}

void extra_Q2_19(void) {
    printf("\nextra_Q2_19: show '1' on a 7-segment display\n");

    // A '1' is the two segments on the right side: number 1 and number 2.
    // Turn on bit 1 and bit 2:
    int value = (1 << 1) | (1 << 2);         // 010 + 100 = 110 = 6
    printf("  value to write: ");
    check(value, 6);

    volatile int* segm = &segm_port19;       // exam: (volatile int*) 0xff34;
    *segm = 6;

    printf("  the display now shows:\n");
    draw7(*segm);
}


int main(void) {
    item2b();
    item4a();
    item7a();
    item10a();
    item13b();
    extra_Q2_14();
    extra_Q2_17();
    extra_Q2_19();
    return 0;
}
