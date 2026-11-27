#include <stdio.h>

// =====================================================================
// Module 6, Q6: AMDAHL'S LAW
//
// THE FORMULA
//
//     speedup = 1 / (s + p / N)        with  s + p = 1
//
//   s = the SEQUENTIAL part: the share of the one-core time that can NOT
//       be made faster with more cores (a number between 0 and 1)
//   p = the PARALLEL part: the share of the one-core time that CAN be
//       split over the cores (p = 1 - s)
//   N = the number of cores
//   speedup = (time on 1 core) / (time on N cores)
//
// The same thing written with seconds instead of shares:
//
//     time on N cores = seq + par / N       where seq + par = time on 1 core
//
// With infinitely many cores, par / N becomes 0. So:
//
//     shortest possible time = seq
//     largest possible speedup = 1 / s      (so s = 1 / max speedup)
//
// Every exam item is its own function (item1b, item2a, ...). Each one
// solves for the unknown one step per line, prints the answer and
// compares it with the exam's answer ("key").
//
// Items 1 to 13 are the 13 exam sittings in date order. The functions
// named extra_Q6_Amdahl_Q.. are questions that exist only as a picture
// and have NO official solution in the pool. For those the program
// checks itself by putting the answer back into the formula.
//
// Run:  gcc -Wall module6_amdahl.c -o amdahl && ./amdahl
// =====================================================================


// ---------- Helpers ----------

// Greatest common divisor: the biggest number that divides both a and b.
// Used to shorten a fraction, for example 64/28 -> 16/7 (gcd is 4).
int gcd(int a, int b) {
    while (b != 0) {
        int rest = a % b;
        a = b;
        b = rest;
    }
    return a;
}

// Compare a decimal answer with the key. Decimals are never exact in a
// computer, so "equal" means "closer than 0.0001".
void check_decimal(double got, double key) {
    double diff = got - key;
    if (diff < 0) {
        diff = -diff;
    }
    printf("  answer = %g   key = %g   ", got, key);
    if (diff < 0.0001) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
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
// Both are shortened first, so 64/28 and 16/7 count as the same.
void check_fraction(int num, int den, int keynum, int keyden) {
    int g = gcd(num, den);
    num = num / g;
    den = den / g;

    int kg = gcd(keynum, keyden);
    keynum = keynum / kg;
    keyden = keyden / kg;

    printf("  answer = %d/%d (= %g)   key = %d/%d   ", num, den, (double)num / den, keynum, keyden);
    if (num == keynum && den == keyden) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}

// Print a fraction in its shortest form, for the items that have no key.
void print_fraction(int num, int den) {
    int g = gcd(num, den);
    printf("%d/%d", num / g, den / g);
}

// You can not buy half a core. Round a number of cores UP to a whole number.
// (6.4 cores -> 7 cores. 6.0 cores -> 6 cores.)
int round_up_cores(double cores) {
    int whole = (int)(cores + 0.000001);   // cut off the decimals (the tiny extra guards against 5.9999999)
    if (whole < cores - 0.000001) {        // was there something after the decimal point?
        whole = whole + 1;                 // then we need one more core
    }
    return whole;
}

// For the items without a key: say whether our own check worked out.
void self_check(double got, double wanted) {
    double diff = got - wanted;
    if (diff < 0) {
        diff = -diff;
    }
    printf("  self-check: formula gives %g, should be %g   ", got, wanted);
    if (diff < 0.0001) {
        printf("OK\n");
    } else {
        printf("WRONG\n");
    }
}


// ===================== Item 1 (b) =====================
// File: 2016-06-09 Q6.pdf
// Question: 1.2 s on one core. Speedup 1.5 on 2 cores. What is the
//           speedup on 4 cores?
// Key: the speedup is 2.
void item1b(void) {
    printf("Item 1 (b): speedup on 4 cores\n");

    double t1 = 1200;          // time on 1 core, in milliseconds (1.2 s)
    double speedup2 = 1.5;     // measured speedup on 2 cores

    // Step 1: time on 2 cores.   speedup = t1 / t2   ->   t2 = t1 / speedup
    double t2 = t1 / speedup2;                 // 1200 / 1.5 = 800 ms

    // Step 2: find the parallel part "par" (in ms).
    //   t2 = seq + par / 2   and   seq = t1 - par
    //   t2 = t1 - par + par / 2
    //   t2 = t1 - par / 2
    //   par = 2 * (t1 - t2)
    double par = 2 * (t1 - t2);                // 2 * 400 = 800 ms

    // Step 3: the sequential part is the rest.
    double seq = t1 - par;                     // 1200 - 800 = 400 ms

    // Step 4: time on 4 cores.
    double t4 = seq + par / 4;                 // 400 + 200 = 600 ms

    // Step 5: speedup on 4 cores.
    double speedup4 = t1 / t4;                 // 1200 / 600 = 2

    printf("  t2 = %g ms, par = %g ms, seq = %g ms, t4 = %g ms\n", t2, par, seq, t4);
    check_decimal(speedup4, 2);
}


// ===================== Item 2 (a) =====================
// File: 2017-01-12 Q6.pdf
// Question: the theoretical maximal speedup is 4. One core takes 20 s.
//           How many percent of the one-core time can be parallelized?
// Key: 75%.
void item2a(void) {
    printf("Item 2 (a): parallel part in percent\n");

    double t1 = 20;            // time on 1 core, seconds
    double max_speedup = 4;    // speedup with infinitely many cores

    // Step 1: with infinitely many cores only the sequential part is left.
    //   max speedup = t1 / seq   ->   seq = t1 / max speedup
    double seq = t1 / max_speedup;             // 20 / 4 = 5 s

    // Step 2: the parallel part is the rest.
    double par = t1 - seq;                     // 20 - 5 = 15 s

    // Step 3: as a share of the whole time, in percent.
    double percent = 100 * par / t1;           // 100 * 15 / 20 = 75

    printf("  seq = %g s, par = %g s\n", seq, par);
    check_decimal(percent, 75);
}


// ===================== Item 3 (b) =====================
// File: 2017-03-16 Q6.pdf
// Question: speedup 3 on 4 cores. Estimate the speedup on 16 cores.
// Key: the speedup would be 6. (The key gets s = 1/9 on the way.)
void item3b(void) {
    printf("Item 3 (b): speedup on 16 cores\n");

    int n = 4;                 // cores we measured on
    int speedup = 3;           // measured speedup
    int m = 16;                // cores we want to know about

    // Step 1: find s (the sequential share) as an exact fraction.
    //   1 / speedup = s + (1 - s) / n          (Amdahl, turned upside down)
    //   n / speedup = s * n + 1 - s            (multiply both sides by n)
    //   n / speedup - 1 = s * (n - 1)
    //   s = (n - speedup) / (speedup * (n - 1))
    int s_num = n - speedup;                   // 4 - 3 = 1
    int s_den = speedup * (n - 1);             // 3 * 3 = 9      -> s = 1/9

    // Step 2: p = 1 - s, with the same denominator.
    int p_num = s_den - s_num;                 // 9 - 1 = 8      -> p = 8/9

    // Step 3: speedup on m cores.
    //   speedup = 1 / (s_num/s_den + p_num/(s_den * m))
    //   multiply top and bottom by s_den * m:
    //   speedup = (s_den * m) / (s_num * m + p_num)
    int num = s_den * m;                       // 9 * 16 = 144
    int den = s_num * m + p_num;               // 16 + 8 = 24    -> 144/24 = 6

    printf("  s = %d/%d, p = %d/%d\n", s_num, s_den, p_num, s_den);
    check_fraction(num, den, 6, 1);
}


// ===================== Item 4 (a) =====================
// File: 2017-06-08 Q6.pdf
// Question: the maximal theoretical speedup is 10. One core takes 40 s.
//           How many cores are needed for a speedup of 4?
// Key: 6 cores are needed.
void item4a(void) {
    printf("Item 4 (a): cores needed for speedup 4\n");

    double t1 = 40;            // time on 1 core, seconds
    double max_speedup = 10;
    double target = 4;         // the speedup we want

    // Step 1: sequential part.   seq = t1 / max speedup
    double seq = t1 / max_speedup;             // 40 / 10 = 4 s

    // Step 2: parallel part.
    double par = t1 - seq;                     // 40 - 4 = 36 s

    // Step 3: the time we must reach.   target = t1 / time  ->  time = t1 / target
    double time_wanted = t1 / target;          // 40 / 4 = 10 s

    // Step 4: how much of that time is left for the parallel part?
    //   time_wanted = seq + par / n   ->   par / n = time_wanted - seq
    double left = time_wanted - seq;           // 10 - 4 = 6 s

    // Step 5: the number of cores.   par / n = left  ->  n = par / left
    double n = par / left;                     // 36 / 6 = 6

    printf("  seq = %g s, par = %g s, time wanted = %g s, n = %g\n", seq, par, time_wanted, n);
    check_whole(round_up_cores(n), 6);
}


// ===================== Item 5 (a) =====================
// File: 2018-01-15 Q6.pdf
// Question: 16 s on one core, 10 s on two cores. What is the expected
//           relative speedup on 4 cores?
// Key: 16/7.
// ("Relative" speedup = compared with your own program on one core.)
void item5a(void) {
    printf("Item 5 (a): relative speedup on 4 cores\n");

    int t1 = 16;               // seconds on 1 core
    int t2 = 10;               // seconds on 2 cores

    // Step 1: find the parallel part "par" in seconds.
    //   t2 = par / 2 + (t1 - par)
    //   t2 = t1 - par / 2
    //   par = 2 * (t1 - t2)
    int par = 2 * (t1 - t2);                   // 2 * 6 = 12 s

    // Step 2: sequential part.
    int seq = t1 - par;                        // 16 - 12 = 4 s

    // Step 3: speedup on 4 cores, kept as a fraction.
    //   speedup = t1 / (seq + par / 4)
    //   multiply top and bottom by 4:
    //   speedup = (4 * t1) / (4 * seq + par)
    int num = 4 * t1;                          // 64
    int den = 4 * seq + par;                   // 16 + 12 = 28    -> 64/28 = 16/7

    printf("  par = %d s, seq = %d s, time on 4 cores = %g s\n", par, seq, seq + par / 4.0);
    check_fraction(num, den, 16, 7);
}


// ===================== Item 6 (b) =====================
// File: 2018-03-15 Q6.pdf
// Question: the sequential part is 1/3 of the one-core time. How many
//           cores for a speedup of 2? For a speedup of 4? Say
//           "impossible" if it can not be done.
// Key: 4 cores give speedup 2. Speedup 4 is impossible (the limit is 3).
// Note: in the PDF text the fraction 1/3 comes out as "31". It is 1/3.

// Helper for this item: returns the cores needed, or -1 for "impossible".
// s_num/s_den is the sequential share, target is the wanted speedup.
int cores_for_speedup(int s_num, int s_den, double target) {
    // Use s_den time units for the whole one-core run (3 units here).
    double t1 = s_den;                         // 3 units
    double seq = s_num;                        // 1 unit
    double par = t1 - seq;                     // 2 units

    // The time we must reach.
    double time_wanted = t1 / target;

    // What is left for the parallel part.
    double left = time_wanted - seq;

    // If nothing (or less than nothing) is left, no number of cores is enough:
    // even infinitely many cores still need the "seq" time.
    if (left <= 0.000001) {
        return -1;
    }

    // n = par / left, rounded up to whole cores.
    return round_up_cores(par / left);
}

void item6b(void) {
    printf("Item 6 (b): cores for speedup 2 and for speedup 4, s = 1/3\n");

    int s_num = 1;             // s = 1/3
    int s_den = 3;

    // The largest speedup there can ever be:  1 / s = s_den / s_num
    double max_speedup = (double)s_den / s_num;        // 3
    printf("  largest possible speedup = 1 / s = %g\n", max_speedup);

    // Speedup 2:  time wanted = 3/2 = 1.5 units, left = 1.5 - 1 = 0.5, n = 2 / 0.5 = 4
    int n2 = cores_for_speedup(s_num, s_den, 2);
    printf("  speedup 2:");
    check_whole(n2, 4);

    // Speedup 4:  time wanted = 3/4 = 0.75 units, but seq alone is 1 unit -> impossible
    int n4 = cores_for_speedup(s_num, s_den, 4);
    if (n4 == -1) {
        printf("  speedup 4:  answer = impossible   key = impossible   OK\n");
    } else {
        printf("  speedup 4:  answer = %d cores   key = impossible   WRONG\n", n4);
    }
}


// ===================== Item 7 (a) =====================
// File: 2018-06-05 Q6.pdf
// Question: part A is sequential. Part B is parallel and is 25% of the
//           time on one core. What is the maximal speedup?
// Key: the maximal speedup is 4/3.
void item7a(void) {
    printf("Item 7 (a): maximal speedup, parallel part 25%%\n");

    int p_percent = 25;        // the parallel part, in percent

    // Step 1: the sequential part in percent.
    int s_percent = 100 - p_percent;           // 75

    // Step 2: with infinitely many cores, p / N becomes 0.
    //   max speedup = 1 / s = 100 / s_percent
    int num = 100;
    int den = s_percent;                       // 100/75 = 4/3

    check_fraction(num, den, 4, 3);
}


// ===================== Item 8 (b) =====================
// File: 2019-01-12-riscv Q6.pdf
// Question: 12 s on one core, 6 s on 4 cores. Estimate the execution
//           time on 8 cores.
// Key: 5 seconds.
void item8b(void) {
    printf("Item 8 (b): execution time on 8 cores\n");

    double t1 = 12;            // seconds on 1 core
    double t4 = 6;             // seconds on 4 cores

    // Step 1: find the parallel part "par" in seconds.
    //   t4 = par / 4 + (t1 - par)
    //   t4 = t1 - par * 3/4
    //   par = (t1 - t4) * 4/3
    double par = (t1 - t4) * 4 / 3;            // 6 * 4/3 = 8 s

    // Step 2: sequential part.
    double seq = t1 - par;                     // 12 - 8 = 4 s

    // Step 3: time on 8 cores.
    double t8 = seq + par / 8;                 // 4 + 1 = 5 s

    printf("  par = %g s, seq = %g s\n", par, seq);
    check_decimal(t8, 5);
}


// ===================== Item 9 (a) =====================
// File: 2019-03-15-riscv Q6.pdf
// Question: 10 s on one core. The theoretical limit for the speedup is 5.
//           What is the speedup on 4 cores?
// Key: 5/2.
// Note: in the PDF text the key comes out as "52". It is the fraction 5/2
//       (the elaborated answer ends in 10/4 = 5/2).
void item9a(void) {
    printf("Item 9 (a): speedup on 4 cores\n");

    int t1 = 10;               // seconds on 1 core
    int max_speedup = 5;

    // Step 1: sequential part.   seq = t1 / max speedup
    int seq = t1 / max_speedup;                // 10 / 5 = 2 s  (divides evenly)

    // Step 2: parallel part.
    int par = t1 - seq;                        // 10 - 2 = 8 s

    // Step 3: speedup on 4 cores, kept as a fraction.
    //   speedup = t1 / (seq + par / 4) = (4 * t1) / (4 * seq + par)
    int num = 4 * t1;                          // 40
    int den = 4 * seq + par;                   // 8 + 8 = 16      -> 40/16 = 5/2

    printf("  seq = %d s, par = %d s, time on 4 cores = %g s\n", seq, par, seq + par / 4.0);
    check_fraction(num, den, 5, 2);
}


// ===================== Item 10 (a) =====================
// File: 2019-06-05-riscv Q6.pdf
// Question: the parallel part is 60% of the time on one core. What is
//           the maximal speedup?
// Key: the maximal speedup is 2.5.
void item10a(void) {
    printf("Item 10 (a): maximal speedup, parallel part 60%%\n");

    double p = 0.6;            // parallel share

    // Step 1: sequential share.
    double s = 1 - p;                          // 0.4

    // Step 2: with infinitely many cores, p / N becomes 0.
    double max_speedup = 1 / s;                // 1 / 0.4 = 2.5

    check_decimal(max_speedup, 2.5);

    // The same answer as an exact fraction: 100 / 40 = 5/2.
    printf("  as a fraction: ");
    print_fraction(100, 40);
    printf("\n");
}


// ===================== Item 11 (a) =====================
// File: 2020-01-13-riscv Q6.pdf
// Question: 6 s on 1 core, 3 s on 4 cores. What is the shortest possible
//           time with infinitely many cores?
// Key: 2 seconds.
void item11a(void) {
    printf("Item 11 (a): shortest time with infinitely many cores\n");

    double t1 = 6;             // seconds on 1 core
    double t4 = 3;             // seconds on 4 cores

    // Step 1: find the sequential part "seq" in seconds.
    //   (t1 - seq) / 4 + seq = t4            (this is the key's equation)
    //   t1 - seq + 4 * seq = 4 * t4          (multiply both sides by 4)
    //   3 * seq = 4 * t4 - t1
    //   seq = (4 * t4 - t1) / 3
    double seq = (4 * t4 - t1) / 3;            // (12 - 6) / 3 = 2 s

    // Step 2: with infinitely many cores the parallel part takes no time,
    //         so only seq is left.
    double shortest = seq;

    printf("  seq = %g s, par = %g s\n", seq, t1 - seq);
    check_decimal(shortest, 2);
}


// ===================== Item 12 (b) =====================
// File: 2020-03-11-riscv Q6.pdf
// Question: the theoretical maximal speedup is 5. One core takes 30 s.
//           How many percent of the one-core time can be parallelized?
// Key: 80%.
void item12b(void) {
    printf("Item 12 (b): parallel part in percent\n");

    double t1 = 30;            // seconds on 1 core
    double max_speedup = 5;

    // Step 1: sequential part.   seq = t1 / max speedup
    double seq = t1 / max_speedup;             // 30 / 5 = 6 s

    // Step 2: parallel part.
    double par = t1 - seq;                     // 30 - 6 = 24 s

    // Step 3: in percent of the whole.
    double percent = 100 * par / t1;           // 100 * 24 / 30 = 80

    printf("  seq = %g s, par = %g s\n", seq, par);
    check_decimal(percent, 80);
}


// ===================== Item 13 (a) =====================
// File: 2020-06-05-riscv Q6.pdf
// Question: speedup 4 on 6 cores. What is the speedup on 32 cores?
//           Give an exact fraction.
// Key: 320/41 (about 7.8). The key gets s = 1/10 on the way.
void item13a(void) {
    printf("Item 13 (a): speedup on 32 cores, exact fraction\n");

    int n = 6;                 // cores we measured on
    int speedup = 4;           // measured speedup
    int m = 32;                // cores we want to know about

    // Step 1: find s as an exact fraction (same algebra as item 3 (b)).
    //   1 / speedup = s + (1 - s) / n
    //   n / speedup = s * n + 1 - s
    //   s = (n - speedup) / (speedup * (n - 1))
    int s_num = n - speedup;                   // 6 - 4 = 2
    int s_den = speedup * (n - 1);             // 4 * 5 = 20     -> s = 2/20 = 1/10

    // Step 2: p = 1 - s, with the same denominator.
    int p_num = s_den - s_num;                 // 20 - 2 = 18    -> p = 18/20 = 9/10

    // Step 3: speedup on m cores.
    //   speedup = 1 / (s_num/s_den + p_num/(s_den * m))
    //           = (s_den * m) / (s_num * m + p_num)
    int num = s_den * m;                       // 20 * 32 = 640
    int den = s_num * m + p_num;               // 64 + 18 = 82   -> 640/82 = 320/41

    printf("  s = ");
    print_fraction(s_num, s_den);
    printf(", p = ");
    print_fraction(p_num, s_den);
    printf("\n");
    check_fraction(num, den, 320, 41);
}


// ===================== extra_Q6_Amdahl_Q14 =====================
// File: Q6-Amdahl-Q14.png (not from one of the 13 sittings)
// Question: 10 s on one core. The speedup approaches 5 with more and
//           more cores but never goes beyond 5. Speedup on 4 cores?
// Key: NO official solution in the pool for this picture.
//      It has the same numbers as item 9 (a), whose key is 5/2.
void extra_Q6_Amdahl_Q14(void) {
    printf("extra_Q6_Amdahl_Q14: speedup on 4 cores\n");

    int t1 = 10;               // seconds on 1 core
    int max_speedup = 5;       // "approaches 5 but never beyond" = the limit

    // Step 1: sequential part.   seq = t1 / max speedup
    int seq = t1 / max_speedup;                // 2 s

    // Step 2: parallel part.
    int par = t1 - seq;                        // 8 s

    // Step 3: speedup on 4 cores = (4 * t1) / (4 * seq + par)
    int num = 4 * t1;                          // 40
    int den = 4 * seq + par;                   // 16              -> 5/2

    printf("  (no own key; compared with item 9 (a), same numbers)\n");
    check_fraction(num, den, 5, 2);
}


// ===================== extra_Q6_Amdahl_Q15 =====================
// File: Q6-Amdahl-Q15.png (not from one of the 13 sittings)
// Question: someone else's sequential program takes 5 s. Your parallel
//           program takes 9 s on one core and can never get more than
//           9 times relative speedup. How many cores for a RELATIVE
//           speedup of at least 4? For a TRUE speedup of at least 4?
//           Answer with whole cores.
//   relative speedup = compared with YOUR OWN program on one core (9 s)
//   true speedup     = compared with the FASTEST sequential program (5 s)
// Key: NO official solution in the pool for this picture.
//      Our answers: 7 cores (relative), 32 cores (true).
void extra_Q6_Amdahl_Q15(void) {
    printf("extra_Q6_Amdahl_Q15: cores for relative and for true speedup 4\n");

    double best_seq_time = 5;  // the other person's sequential program, seconds
    double t1 = 9;             // our program on 1 core, seconds
    double max_speedup = 9;    // our limit for the relative speedup
    double target = 4;

    // Step 1: our sequential part.   seq = t1 / max speedup
    double seq = t1 / max_speedup;             // 9 / 9 = 1 s

    // Step 2: our parallel part.
    double par = t1 - seq;                     // 9 - 1 = 8 s

    // ---- Relative speedup 4: compare with our own 9 s ----
    // Step 3: time we must reach.
    double time_rel = t1 / target;             // 9 / 4 = 2.25 s
    // Step 4: what is left for the parallel part.
    double left_rel = time_rel - seq;          // 2.25 - 1 = 1.25 s
    // Step 5: cores.   n = par / left
    double n_rel = par / left_rel;             // 8 / 1.25 = 6.4  -> 7 whole cores
    int cores_rel = round_up_cores(n_rel);

    // ---- True speedup 4: compare with the other program's 5 s ----
    // Step 6: time we must reach.
    double time_true = best_seq_time / target; // 5 / 4 = 1.25 s
    // Step 7: what is left for the parallel part.
    double left_true = time_true - seq;        // 1.25 - 1 = 0.25 s
    // Step 8: cores.
    double n_true = par / left_true;           // 8 / 0.25 = 32
    int cores_true = round_up_cores(n_true);

    printf("  seq = %g s, par = %g s\n", seq, par);
    printf("  relative: n = %g  ->  answer = %d cores   key = (none in the pool)\n", n_rel, cores_rel);
    printf("  true:     n = %g  ->  answer = %d cores   key = (none in the pool)\n", n_true, cores_true);

    // Check ourselves: put the answers back in.
    // 6 cores must NOT be enough for relative speedup 4, 7 cores must be.
    printf("  relative speedup on 6 cores = %g (too little), on 7 cores = %g (enough)\n",
           t1 / (seq + par / 6), t1 / (seq + par / 7));
    // 32 cores must give exactly true speedup 4.
    self_check(best_seq_time / (seq + par / cores_true), 4);
}


// ===================== extra_Q6_Amdahl_Q16 =====================
// File: Q6-Amdahl-Q16.png (not from one of the 13 sittings)
// Question: a robot program has two tasks that run after each other.
//           Task 1 (image analysis) is perfectly parallel. Task 2
//           (decision) always takes 10 ms. On 8 cores the speedup is 4.
//           What is the total time on only one core?
// Key: NO official solution in the pool for this picture.
//      Our answer: 70 ms.
void extra_Q6_Amdahl_Q16(void) {
    printf("extra_Q6_Amdahl_Q16: total time on one core\n");

    double seq = 10;           // the decision task, ms. This is the fixed sequential part.
    double n = 8;              // cores
    double speedup = 4;        // speedup on 8 cores

    // Step 1: call the image task's one-core time "par". Amdahl says:
    //   speedup = (par + seq) / (par / n + seq)
    //   speedup * (par / n + seq) = par + seq
    //   speedup * par / n + speedup * seq = par + seq
    //   speedup * seq - seq = par - speedup * par / n
    //   seq * (speedup - 1) = par * (1 - speedup / n)
    //   par = seq * (speedup - 1) / (1 - speedup / n)
    double par = seq * (speedup - 1) / (1 - speedup / n);   // 10 * 3 / 0.5 = 60 ms

    // Step 2: total time on one core.
    double t1 = par + seq;                     // 60 + 10 = 70 ms

    printf("  image task on one core = %g ms\n", par);
    printf("  answer = %g ms   key = (none in the pool)\n", t1);

    // Check ourselves: 70 ms on one core, 60/8 + 10 = 17.5 ms on 8 cores, 70 / 17.5 = 4.
    self_check(t1 / (par / n + seq), speedup);
}


// ===================== extra_Q6_Amdahl_Q17 =====================
// File: Q6-Amdahl-Q17.png (not from one of the 13 sittings)
// Question: part A is sequential. Part B is parallel and is 20% of the
//           time on one core. What is the maximal speedup?
//           (Same as item 7 (a) but with 20% instead of 25%.)
// Key: NO official solution in the pool for this picture.
//      Our answer: 5/4 = 1.25.
void extra_Q6_Amdahl_Q17(void) {
    printf("extra_Q6_Amdahl_Q17: maximal speedup, parallel part 20%%\n");

    int p_percent = 20;

    // Step 1: sequential part in percent.
    int s_percent = 100 - p_percent;           // 80

    // Step 2: max speedup = 1 / s = 100 / s_percent
    int num = 100;
    int den = s_percent;                       // 100/80 = 5/4

    printf("  answer = ");
    print_fraction(num, den);
    printf(" (= %g)   key = (none in the pool)\n", (double)num / den);

    // Check ourselves: with a huge number of cores the formula must give almost the same.
    double s = s_percent / 100.0;
    double p = p_percent / 100.0;
    self_check(1 / (s + p / 1000000000.0), (double)num / den);
}


// ===================== extra_Q6_Amdahl_Q18 =====================
// File: Q6-Amdahl-Q18.png (not from one of the 13 sittings)
// Question: part A is parallel, part B is sequential. The theoretical
//           maximal speedup is 5. A competitor's sequential program
//           takes 16 s. Ours takes 20 s on one core. What are the
//           relative speedup and the true speedup on 4 cores?
// Key: NO official solution in the pool for this picture.
//      Our answers: relative speedup 5/2 = 2.5, true speedup 2.
void extra_Q6_Amdahl_Q18(void) {
    printf("extra_Q6_Amdahl_Q18: relative and true speedup on 4 cores\n");

    double best_seq_time = 16; // the competitor's sequential program, seconds
    double t1 = 20;            // our program on 1 core, seconds
    double max_speedup = 5;

    // Step 1: our sequential part (part B).   seq = t1 / max speedup
    double seq = t1 / max_speedup;             // 20 / 5 = 4 s

    // Step 2: our parallel part (part A).
    double par = t1 - seq;                     // 20 - 4 = 16 s

    // Step 3: our time on 4 cores.
    double t4 = seq + par / 4;                 // 4 + 4 = 8 s

    // Step 4: relative speedup = our own one-core time / our 4-core time.
    double relative = t1 / t4;                 // 20 / 8 = 2.5

    // Step 5: true speedup = fastest sequential time / our 4-core time.
    double true_speedup = best_seq_time / t4;  // 16 / 8 = 2

    printf("  seq = %g s, par = %g s, time on 4 cores = %g s\n", seq, par, t4);
    printf("  relative speedup: answer = %g (= ", relative);
    print_fraction(20, 8);
    printf(")   key = (none in the pool)\n");
    printf("  true speedup:     answer = %g   key = (none in the pool)\n", true_speedup);

    // Check ourselves: the limit of our relative speedup must be 5 (t1 / seq).
    self_check(t1 / seq, max_speedup);
}


// ===================== extra_Q6_Amdahl_Q19 =====================
// File: Q6-Amdahl-Q19.png (not from one of the 13 sittings)
// Question: 10 cores. We want 5 times speedup. One core takes 100 s.
//           How many percent of the one-core run must be parallel?
//           Answer as a rational number (like 100/3 %).
// Key: NO official solution in the pool for this picture.
//      Our answer: 800/9 % (about 88.9%).
void extra_Q6_Amdahl_Q19(void) {
    printf("extra_Q6_Amdahl_Q19: parallel part in percent, exact fraction\n");

    int n = 10;                // cores
    int speedup = 5;           // the speedup we want
    // (The 100 s is not needed: only the share matters, not the seconds.)

    // Step 1: solve Amdahl for p.
    //   1 / speedup = (1 - p) + p / n
    //   1 / speedup = 1 - p * (n - 1) / n
    //   p * (n - 1) / n = 1 - 1 / speedup = (speedup - 1) / speedup
    //   p = n * (speedup - 1) / (speedup * (n - 1))
    int p_num = n * (speedup - 1);             // 10 * 4 = 40
    int p_den = speedup * (n - 1);             // 5 * 9 = 45      -> p = 40/45 = 8/9

    // Step 2: in percent, multiply by 100.
    int percent_num = 100 * p_num;             // 4000
    int percent_den = p_den;                   // 45              -> 4000/45 = 800/9

    printf("  p = ");
    print_fraction(p_num, p_den);
    printf("\n  answer = ");
    print_fraction(percent_num, percent_den);
    printf(" %% (= %g%%)   key = (none in the pool)\n", (double)percent_num / percent_den);

    // Check ourselves: with p = 8/9 the speedup on 10 cores must be 5.
    double p = (double)p_num / p_den;
    self_check(1 / ((1 - p) + p / n), speedup);
}


int main(void) {
    item1b();
    printf("\n");
    item2a();
    printf("\n");
    item3b();
    printf("\n");
    item4a();
    printf("\n");
    item5a();
    printf("\n");
    item6b();
    printf("\n");
    item7a();
    printf("\n");
    item8b();
    printf("\n");
    item9a();
    printf("\n");
    item10a();
    printf("\n");
    item11a();
    printf("\n");
    item12b();
    printf("\n");
    item13a();
    printf("\n");
    extra_Q6_Amdahl_Q14();
    printf("\n");
    extra_Q6_Amdahl_Q15();
    printf("\n");
    extra_Q6_Amdahl_Q16();
    printf("\n");
    extra_Q6_Amdahl_Q17();
    printf("\n");
    extra_Q6_Amdahl_Q18();
    printf("\n");
    extra_Q6_Amdahl_Q19();
    return 0;
}
