#include <stdio.h>

// ===================== Item 8 =====================
void item8(void) {
    int a = 3;
    int b = 2;
    int lst[] = {6, 13, 19, -7, -4, 18, 4, 23};
    int *p1 = &a;
    int *p2 = lst;

    printf("%x\n", (int)p2);

    b = *(p2 + *p1 * b) * a;
    *p1 = *p1 + p2[3];
    p2++;

    printf("%x, %d, %d, %d\n", (int)p2, a, b, *p2);
}

// ===================== Item 9 =====================
int lst[] = {7, 8, 10, 0};

void foo(int *p) {
    while (*p != 0) {
        // ---------- ADDED (the two missing lines) ----------
        *p = *p + 2;   // code line 1: lw t0,0(a0) / addi t0,t0,2 / sw t0,0(a0)
        p++;           // code line 2: addi a0,a0,4
        // ----------------------------------------------------
    }
}

void bar() {
    int *b = lst;
    foo(b);
}

void item9(void) {
    bar();

    // ---------- ADDED (not in the exam): print lst to check the answer ----------
    printf("{%d, %d, %d, %d}\n", lst[0], lst[1], lst[2], lst[3]);

    // ---------- ADDED (not in the exam): print each element's address in hex ----------
    int *b = lst;   // b points at lst[0]
    printf("{%x, %x, %x, %x}\n", (int)b, (int)(b + 1), (int)(b + 2), (int)(b + 3));
    // -------------------------------------------------------------------------------------
}

// ===================== Item 10 =====================
void item10(void) {
    int x = 4;
    int y = 15;
    int *k = &x;
    int z;
    int *p;
    *k = *k * (*k + 1);
    printf("%d\n", *k);   // ---------- ADDED (not in the exam): check *k ----------
    y = x + y;
    printf("%d\n", y);
    k = &z;

    int a[] = {7, 9, 3, 8};
    p = a;
    printf("%x\n", (unsigned int)p);

    // ---------- NOTE (not in the exam) ----------
    // x IS UNKNOWN ON THE EXAM. z has no value yet here (z = *p - 4 is further down),
    // so *k reads garbage. The 1 printed below happens on every run of this file
    // because z holds 0 at that point, but C does not guarantee that value.
    // ----------------------------------------------
    x = *k + 1;
    printf("%d\n", x);
    p = p + 2;
    z = *p - 4;

    printf("%d %d %x %x\n", x, y, z, (unsigned int)p);

    // ---------- ADDED (not in the exam): address of z, two ways to cast ----------
    printf("%x\n", (int)&z);
    printf("%x\n", (unsigned int)&z);
    // -------------------------------------------------------------------------------
}

// ===================== Item 11 =====================
int a[] = {7, 10, 3, 5, 12, 8, 17, -8, 4, 5};
int x[] = {8, 2, 0};

int sum(int* p1, int* p2, int len) {
    int i = 0;
    int s = 0;
    while (i < len) {
        // ---------- ADDED (the missing code) ----------
        s += *(p1 + *p2);   // *p2 is the index (8, then 2, then 0); p1 + index points at that element
        p2++;               // move p2 to the next index
        i++;                // count the iteration so the loop stops after len (3) rounds
        // -----------------------------------------------
    }
    return s;
}

void item11(void) {
    int v = sum(a, x, 3);
    printf("Sum %d\n", v);
}

// ===================== Item 12 =====================
// ---------- ADDED (the whole function is the answer) ----------
// Named foo12 here only because item 9 already has a foo. On the exam, call it foo.
void foo12(int* p, int v) {   // int pointer + int value, returns nothing (void)
    while (*p != 0) {         // stop at the element that is 0 (the "null terminator")
        *p = *p + v;          // add v to the element p points to
        p++;                  // move p to the next element (4 bytes on)
    }
}
// ----------------------------------------------------------------
void foo1122(int* p, int v, int buflen) {   // int pointer + int value + int length, returns nothing (void)
    
    for (int i = 0; i < buflen; i++) {
        *p = *p + v;
        p++;
    }
    p--;
    *p = 0;

}
void item12(void) {
    int a[] = {4, 10, 100, 23, 99, 0};
    foo12(a, 10);

    // ---------- ADDED (not in the exam): print a to check the answer ----------
    printf("%d,%d,%d,%d,%d,%d\n", a[0], a[1], a[2], a[3], a[4], a[5]);

    int b[] = {4, 10, 100, 23, 99, 0};
    int buflen = sizeof(b) / sizeof(int);
    foo1122(b, 10, buflen);
    printf("%d,%d,%d,%d,%d,%d\n", b[0], b[1], b[2], b[3], b[4], b[5]);
}


// ===================== Item 13 =====================
void something(int* foo, int len, int v)
{
    int i;
    for (i = 0; i < len; i++) {
        // ---------- ADDED (the two missing statements) ----------
        *foo = *foo * v;   // statement #1: multiply the element foo points to by v
        foo++;             // statement #2: move foo to the next element
        // ----------------------------------------------------------
    }
}

// ---------- ADDED (complementary version of something, same idea as foo12) ----------
// Stops at the 0 at the end of the list instead of using len.
void something2(int* foo, int v) {
    while (*foo != 0) {          // stop at the 0 (the end marker)
        *foo = *foo * v;         // multiply the element foo points to by v
        foo++;                   // move to the next element
    }
}
// -----------------------------------------------------------------------------

void item13(void) {
    int k1[] = {2, 2, 3, 3, 10};
    int len = sizeof(k1) / sizeof(int);   // like buflen in item 3: total bytes / bytes per int = 20 / 4 = 5 elements
    something(k1, len, 4);

    // ---------- ADDED (not in the exam): print k1 to check the answer ----------
    printf("{%d,%d,%d,%d,%d}\n", k1[0], k1[1], k1[2], k1[3], k1[4]);

    // ---------- ADDED (complementary version): k2 = k1 copied, with a 0 appended ----------
    int k2[] = {2, 2, 3, 3, 10, 0};
    something2(k2, 4);
    printf("{%d,%d,%d,%d,%d}\n", k2[0], k2[1], k2[2], k2[3], k2[4]);
}

// ===================== Item 4 =====================
void item4(void) {
    int v1 = 12;
    int a[] = {7, 8, 10, 2, 22, 15};
    int *p1 = &v1;
    int *p2 = a;
    int v2 = *p1 + *(p2+3) + v1;
    *p1 = (v1 + 10) | 1;
    p2++;
    p1 = p2 + 5;
    // ---------- NOTE (not in the exam) ----------
    // *p1 IS UNKNOWN ON THE EXAM. p1 = p2 + 5 points past the end of a (a has only
    // indices 0 to 5, and p2 is at a[1], so p1 is at a[6]). Reading it is undefined.
    // (int)p2 IS UNKNOWN ON THE EXAM. The address of a is not given in the question.
    // ----------------------------------------------
    printf("%d, %d, %d, %d, %d\n", v1, v2, *p1, *p2, (int)p2);
}

// ===================== Item 3 =====================
// Named sum3 here only because item 11 already has a sum. On the exam, call it sum.
void sum3(int* buf, int *res, int len) {
    int i;
    int s = 0;
    for (i = 0; i < len; i++) {
        // ---------- ADDED (code lines 1 and 2) ----------
        s = s + *buf;   // code line 1: read the element buf points to, add it to s
        buf++;          // code line 2: move buf to the next element
        // --------------------------------------------------
    }
    *res = s;
}

void item3(void) {
    int data[] = {3, 7, 20, 15, 5};
    int res;
    int *bufptr = data;
    int buflen = sizeof(data) / sizeof(int);

    // ---------- ADDED (code line 3) ----------
    sum3(bufptr, &res, buflen);   // &res so sum can write the result into res
    // -------------------------------------------

    printf("Sum: %d\n", res);

    // ---------- ADDED (not in the exam): sizeof ----------
    printf("%zu, %zu, %d\n", sizeof(data), sizeof(int), buflen);
    // -------------------------------------------------------
}

// ===================== Item 2 =====================
void item2(void) {
    int a = 10;
    int b = 20;
    int c = 30;
    int d;
    int *e = &b;
    int k[] = {3, 5, 6, 10, 3, -8};
    int *f = &k[0];
    int *g = &c;
    a += *(f + 5);
    b = *e << 3;
    e = g;
    // ---------- NOTE (not in the exam) ----------
    // k[2] AND c ARE UNKNOWN ON THE EXAM. d has no value yet here (d = *e comes below),
    // so d + a reads garbage. c = k[2] + a depends on that, so c is unknown too.
    // THE ADDRESS IN e IS UNKNOWN ON THE EXAM. e = g makes e point at c, but the
    // address of c is not given in the question (it also changes on every run).
    // ----------------------------------------------
    k[2] = d + a;
    d = *e;
    c = k[2] + a;

    // ---------- ADDED (not in the exam): print the answers ----------
    printf("a = %d, b = %d, c = %d, d = %d\n", a, b, c, d);
    printf("e = %p, &c = %p\n", e, &c);
}

// ===================== Item 14 =====================
void item14(void) {
    int a = 10;
    int* b = &a;
    a++;
    int c = a + *b;
    char* d = "HELLO";
    // ---------- ADDED (not in the exam): what "HELLO" really is ----------
    // "HELLO" is a hidden, read-only char array: {'H', 'E', 'L', 'L', 'O', '\0'}
    // and d points at its 'H'. For READING (as here) this gives the same answer:
    //     char s[] = {'H', 'E', 'L', 'L', 'O', '\0'};
    //     char* d = s;
    // Not allowed:  char* d = {'H', 'E', 'L', 'L', 'O', '\0'};  (a {...} list is not an array)
    // ---------------------------------------------------------------------
    char* e = d + 4;
    a = c + (int)*e + (int)d[5];
    printf("%d\n", a);
}

// ===================== Item 15 =====================
void item15(void) {
    // In the exam, v and a are global (outside main). They are inside this
    // function here only because item 11 already has a global called a.
    double v[4] = {2.5, 5.4, 3.3, 8.7};
    double a = 2.0;

    double* p1 = v;
    double* p2 = &a;
    for (int i = 0; i < 4; i++) {
        *p1 = *p1 + *p2;
        a = *p1;
        p1 = p1 + 1;
    }
    printf("%0.1f %d\n", v[1], (int)sizeof(v));
}

// ========== Item 1 (SPECIAL: translate ASSEMBLY into C) ==========
// Here no C code is given at all: you write the whole C function from assembly.
// Named foo_FROM_ASSEMBLY only because item 9 already has a foo. On the exam, call it foo.
//
//   MIPS (this question)       RISC-V (the exam's version)
//   lw   $v0,0($a0)            lw  t0,0(a0)
//   add  $v0,$v0,$a1           add t0,t0,a1
//   sw   $v0,0($a0)            sw  t0,0(a0)
//                              add a0,t0,zero
//   jr   $ra                   jr  ra

// ---------- ADDED (the whole function is the answer) ----------
int foo_FROM_ASSEMBLY(int* x, int y) {   // a0 is used as an address -> int* x; a1 is a number -> int y
    int tmp = *x + y;                    // lw: read *x;  add: + y
    *x = tmp;                            // sw: write the sum back to where x points
    return tmp;                          // $v0 (MIPS) / a0 (RISC-V) holds the return value; jr ra returns
}
// ----------------------------------------------------------------

void item1_SPECIAL_assembly_to_c(void) {
    // ---------- ADDED (not in the exam): call foo to check it ----------
    int n = 5;
    int r = foo_FROM_ASSEMBLY(&n, 3);
    printf("returned %d, n is now %d\n", r, n);
}

int main() {
//    item8();
//    item9();
//    item10();
//    item11();
//    item12();
//    item13();
    item4();
//    item3();
//    item2();
//    item14();
//    item15();
//    item1_SPECIAL_assembly_to_c();
}
