#include <stdio.h>

// =====================================================================
// Module 2, Q2: I/O SYSTEMS - the TIMER ("clock") questions
//
// Every exam item is its own function (item1b, item3c, ...).
// Each one works out the answer step by step, prints it, and compares
// it with the exam's answer ("key").
//
// How a timer works (this is all you need for every item here):
//   - The timer is a counter. It goes up by 1 on every clock tick.
//   - clock frequency = how many ticks there are in one second.
//       8 MHz = 8 000 000 ticks every second.
//   - PRESCALE 1:N  = the timer only counts every Nth tick, so it runs
//       N times slower.
//   - PERIOD REGISTER = the value where the timer "wraps": it goes back
//       to 0 and (if set up) triggers an interrupt.
//   - An n-bit timer can hold at most 2^n - 1.  16 bits -> 65 535.
//
//   The one formula:
//       wraps per second = frequency / (prescale * period)
//   or the other way round:
//       period = frequency * seconds / prescale
//
// Items 1 to 7 come from old MIPS exams, but timer questions are pure
// arithmetic, so nothing changes for RISC-V.
//
// Run:  gcc -Wall module2_timer.c -o timer && ./timer
// =====================================================================


// ---------- Helpers ----------

// Compare our number with the exam's answer.
void check(int got, int key) {
    printf("  answer = %d   key = %d   ", got, key);
    if (got == key) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// The biggest number an n-bit register can hold: 2^n - 1.
// 1 << n is 2^n. (1u is the number 1 as an unsigned int. Only used here
// for n below 32.)
unsigned int maxValue(int bits) {
    return (1u << bits) - 1;
}


// ===================== Item 1 (b) =====================
// 2016-06-09 Q2.pdf
// 16-bit timer, clock 8 MHz, prescale options 1:32, 1:8, 1:2, 1:1.
// Choose a period register value and a prescale so that the interrupt
// comes every 10 ms.
// Key: period 40 000 with prescale 1:2.  (Also right: 10 000 with 1:8.
//      The key says there are several correct answers.)
void item1b(void) {
    printf("\nItem 1(b): 16-bit timer, 8 MHz, interrupt every 10 ms\n");

    int frequency = 8000000;                 // ticks per second
    int ms = 10;                             // wanted time between interrupts

    // Step 1: how many clock ticks are there in 10 ms?
    int ticks = frequency / 1000 * ms;       // 8 000 ticks per ms * 10 = 80 000
    printf("  clock ticks in %d ms = %d\n", ms, ticks);

    // Step 2: 80 000 does not fit in 16 bits (max 65 535), so we need a prescale.
    int max = maxValue(16);
    printf("  biggest 16-bit value = %d\n", max);

    // Step 3: try every prescale. period = ticks / prescale.
    int prescale[] = {1, 2, 8, 32};
    int period12 = 0;                        // the period for 1:2 (the key's choice)
    int period18 = 0;                        // the period for 1:8 (the key's other choice)
    for (int i = 0; i < 4; i++) {
        int period = ticks / prescale[i];
        printf("  prescale 1:%-2d -> period = %5d   ", prescale[i], period);
        if (period <= max) {
            printf("fits in 16 bits\n");
        } else {
            printf("TOO BIG for 16 bits\n");
        }
        if (prescale[i] == 2) period12 = period;
        if (prescale[i] == 8) period18 = period;
    }

    printf("  with prescale 1:2:\n");
    check(period12, 40000);
    printf("  with prescale 1:8:\n");
    check(period18, 10000);
    // (1:32 with period 2 500 works too. Only 1:1 is impossible.)
}


// ===================== Item 3 (c) =====================
// 2017-03-16 Q2.pdf
// 16-bit timer, prescale 1:4, period register 30 000.
// The timer triggers every millisecond. What is the clock frequency?
// Key: 120 MHz
void item3c(void) {
    printf("\nItem 3(c): prescale 1:4, period 30 000, triggers every 1 ms. Frequency?\n");

    int prescale = 4;
    int period = 30000;

    // Every millisecond = 1000 times every second.
    int wrapsPerSecond = 1000;

    // One wrap needs  period * prescale  clock ticks.
    int ticksPerWrap = period * prescale;            // 120 000
    printf("  clock ticks for one wrap = %d * %d = %d\n", period, prescale, ticksPerWrap);

    // frequency = ticks per second = ticks for one wrap * wraps per second
    int frequency = ticksPerWrap * wrapsPerSecond;   // 120 000 000 Hz
    printf("  frequency = %d * %d = %d Hz\n", ticksPerWrap, wrapsPerSecond, frequency);

    int mhz = frequency / 1000000;
    printf("  in MHz:\n");
    check(mhz, 120);
}


// ===================== Item 6 (b) =====================
// 2018-03-15 Q2.pdf
// 16-bit timer, 4 MHz, prescale 1:8, period register 1000.
// Every time the timer wraps, a LED is toggled (on, off, on, off ...).
//   - How many times per second is the LED turned ON?
//   - How should the prescale change to make it twinkle twice as fast?
// Key: 250 times per second. Prescale 1:4.
void item6b(void) {
    printf("\nItem 6(b): 4 MHz, prescale 1:8, period 1000, LED toggles on every wrap\n");

    int frequency = 4000000;
    int prescale = 8;
    int period = 1000;

    // Step 1: the prescale slows the clock down.
    int countsPerSecond = frequency / prescale;      // 500 000
    printf("  timer counts per second = %d / %d = %d\n", frequency, prescale, countsPerSecond);

    // Step 2: the timer wraps every 1000 counts.
    int wraps = countsPerSecond / period;            // 500
    printf("  wraps per second = %d / %d = %d\n", countsPerSecond, period, wraps);

    // Step 3: every wrap toggles the LED. Only every SECOND toggle turns it ON.
    int turnedOn = wraps / 2;                        // 250
    printf("  times the LED is turned ON each second:\n");
    check(turnedOn, 250);
    // (The key gives 1 point out of 2 for answering 500: that is the number
    //  of wraps, not the number of times the LED is turned on.)

    // Step 4: twice as fast = the timer must count twice as fast
    //         = half the prescale.
    int newPrescale = prescale / 2;                  // 4
    printf("  new prescale 1:N, N:\n");
    check(newPrescale, 4);

    // Check it: with 1:4 the LED is turned on 500 times, twice as many as 250.
    int newTurnedOn = frequency / newPrescale / period / 2;
    printf("  with prescale 1:%d the LED is turned on %d times per second\n", newPrescale, newTurnedOn);
}


// ===================== Item 8 (b) =====================
// 2019-01-12-riscv Q2.pdf   (same as Q2.1clock.png)
// 32-bit timer, clock 25 MHz. What value should the period register
// have for a period of 3 seconds?
// Key: 75 000 000
void item8b(void) {
    printf("\nItem 8(b): 32-bit timer, 25 MHz, period of 3 seconds\n");

    int frequency = 25000000;        // ticks per second
    int seconds = 3;

    // No prescale here. The timer must count for 3 seconds of ticks.
    int period = frequency * seconds;
    printf("  period = %d * %d\n", frequency, seconds);
    check(period, 75000000);

    // A 32-bit register can hold up to about 4 294 000 000, so it fits.
    printf("  biggest 32-bit value = %u, so it fits\n", 0xffffffffu);
}


// ===================== Item 9 (a) i =====================
// 2019-03-15-riscv Q2.pdf   (same as Q2.2clock.png)
// True or false: a 16-bit timer runs at 100 MHz. You want to print a
// message every 10th second. The period register cannot be set so that
// the timer triggers every 10th second. But it is still possible to do
// it with the 16-bit timer by using software counters.
// Key: True.
// (The other three statements of this question are in module2_theory.md.)
void item9a_i(void) {
    printf("\nItem 9(a) i: 16-bit timer at 100 MHz, a message every 10 seconds. Possible?\n");

    int frequency = 100000000;       // ticks per second
    int seconds = 10;
    int max = maxValue(16);          // 65 535

    // Part 1: can the period register do it alone?
    int ticks = frequency * seconds; // 1 000 000 000 ticks in 10 seconds
    printf("  ticks in %d s = %d, biggest 16-bit value = %d\n", seconds, ticks, max);
    int alone = (ticks <= max);      // 1 = yes, 0 = no
    printf("  the period register alone is enough: %s\n", alone ? "yes" : "no");

    // Part 2: a SOFTWARE COUNTER is an ordinary variable that the program
    // adds 1 to every time the timer wraps. Pick a period that fits and
    // count the wraps.
    int period = 50000;                          // fits in 16 bits
    int wrapsPerSecond = frequency / period;     // 2 000
    int count = wrapsPerSecond * seconds;        // 20 000 wraps = 10 seconds
    printf("  period %d -> %d wraps per second -> count to %d for %d s\n",
           period, wrapsPerSecond, count, seconds);

    // The statement is true if the timer alone is NOT enough but a
    // software counter makes it work (the period fits and count is a whole number).
    int statement = (!alone) && (period <= max) && (count * period == ticks);
    printf("  statement is %s   key = True   ", statement ? "True" : "False");
    if (statement == 1) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 11 (b) =====================
// 2020-01-13-riscv Q2.pdf   (same as Q2.4clock.png)
// 32-bit timer, clock 4 MHz, period register 500 000. A C variable
// "count" counts how many times the timer has expired. What value
// should count reach to measure one second?
// Key: 8
void item11b(void) {
    printf("\nItem 11(b): 4 MHz, period 500 000. count for one second?\n");

    int frequency = 4000000;
    int period = 500000;

    // The timer expires every 500 000 ticks. One second is 4 000 000 ticks.
    int count = frequency / period;
    printf("  count = %d / %d\n", frequency, period);
    check(count, 8);

    // The same thing in time: one expiry takes 500 000 / 4 000 000 = 0.125 s,
    // and 8 * 0.125 s = 1 s. In whole milliseconds:
    int msPerExpiry = period / (frequency / 1000);   // 500 000 / 4 000 = 125 ms
    printf("  one expiry = %d ms, %d expiries = %d ms\n", msPerExpiry, count, msPerExpiry * count);
}


// ===================== Item 12 (a) =====================
// 2020-03-11-riscv Q2.pdf   (same as Q2.5clock.png)
// 16-bit timer, clock 10 MHz. The timer should have a period of 5 ms,
// that is, wrap 200 times every second. Period register value?
// Key: 50 000
void item12a(void) {
    printf("\nItem 12(a): 16-bit timer, 10 MHz, wraps 200 times per second\n");

    int frequency = 10000000;
    int wrapsPerSecond = 200;

    int period = frequency / wrapsPerSecond;
    printf("  period = %d / %d\n", frequency, wrapsPerSecond);
    check(period, 50000);

    // It must fit in 16 bits.
    int max = maxValue(16);
    printf("  biggest 16-bit value = %d, so 50 000 %s\n", max, period <= max ? "fits" : "does NOT fit");

    // The key also writes it in binary: ... 1100 0011 0101 0000 = 0xc350.
    printf("  in hex: 0x%x\n", period);
}


// ===================== Item 13 (a) =====================
// 2020-06-05-riscv Q2.pdf   (same as Q2.6clock.png)
// 20-bit timer, clock 20 MHz. Choose a period register value so that
// the interrupt is triggered every 10 ms.
// Key: 200 000
void item13a(void) {
    printf("\nItem 13(a): 20-bit timer, 20 MHz, interrupt every 10 ms\n");

    int frequency = 20000000;
    int ms = 10;

    int period = frequency / 1000 * ms;      // 20 000 ticks per ms * 10
    printf("  period = %d / 1000 * %d\n", frequency, ms);
    check(period, 200000);

    // A 20-bit register holds up to 2^20 - 1 = 1 048 575, so no prescale is needed.
    int max = maxValue(20);
    printf("  biggest 20-bit value = %d, so 200 000 %s\n", max, period <= max ? "fits" : "does NOT fit");

    // Check it the key's way: 20 000 000 / 200 000 = 100 interrupts per second = every 10 ms.
    printf("  interrupts per second = %d\n", frequency / period);
}


// One more timer calculation (8-bit timer, 1 MHz, 1 ms period, choose the
// prescale) is part of a "write the C code" question. It is in
// module2_io.c, item extra_Q2_14.

int main(void) {
    item1b();
    item3c();
    item6b();
    item8b();
    item9a_i();
    item11b();
    item12a();
    item13a();
    return 0;
}
