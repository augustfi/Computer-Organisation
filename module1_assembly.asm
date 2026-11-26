# =====================================================================
# Module 1, second part of Q1: RISC-V ASSEMBLY and MACHINE CODE
#
# Every exam item is its own function (item1a, item1b, ...).
# main calls them one after another. To run only some items, put a #
# in front of the calls you do not want (just like commenting out the
# calls in main() in the C file).
#
# The instruction the exam asks about is marked  <-- EXAM  and its
# machine code is written beside it. Each item prints its result, with
# the exam's answer ("key") in the text.
#
# Items 1 to 7 come from old MIPS exams. They are written here in their
# RISC-V version.
#
# Run in RARS (a RISC-V simulator):
#   1. Open this file in RARS.
#   2. Assemble (F3). The "Code" column shows each instruction's machine code.
#   3. Run (F5). The results are printed in the "Run I/O" box.
# From the terminal:  java -jar rars1_6.jar module1_assembly.asm
# =====================================================================

.data
buffer:  .word 0, 0, 0, 0          # 16 bytes of memory for the sw / lw / lb items
word5:   .word 0xff5fab03          # item 5: the machine code, stored as data
n:       .word 5                   # item 1(a): the int that foo changes
lst:     .word 7, 8, 10, 0         # item 9(b): int lst[] = {7, 8, 10, 0};

s1a:  .string "Item 1(a)  foo(&n, 3) returns (key 8): "
s1b:  .string "Item 1(b)  sw t3,-8(a0): memory now holds (key 0x00001234): "
s2a:  .string "Item 2(a)  addi t3,s0,-7 with s0 = 10: t3 = (key 3): "
s3b:  .string "Item 3(b)  lw s1,8(t0): s1 = (key 77): "
s4a:  .string "Item 4(a)  xor s0,t2,t3: s0 = (key 0x12a3ff11): "
s5b:  .string "Item 5(b)  bytes at address +0, +1, +2, +3 (key 03, ab, 5f, ff):"
s6b:  .string "Item 6(b)  after the loop t1 = (key 0): "
s7a:  .string "Item 7(a)  addi t3,t6,-23 with t6 = 100: t3 = (key 77): "
s8a:  .string "Item 8(a)  sll t2,t0,s1: t2 = (key 48): "
s9a:  .string "Item 9(a)  sra t0,t1,t2: t0 = (key -8): "
s9b:  .string "Item 9(b)  lst after bar (key 9, 10, 12, 0):"
s10b: .string "Item 10(b) lui t0,0x32a15: t0 = (key 0x32a15000): "
s11b: .string "Item 11(b) lb t3,-11(s2) reads byte 0xf0: t3 = (key -16): "
s12a: .string "Item 12(a) sw s3,-8(a1): memory now holds (key 99): "
s13b: .string "Item 13(b) the loop ran s3 times (key 110): "

.text

# =====================================================================
# main: put a # in front of a call to skip that item
# =====================================================================
main:
        jal  item1a
        jal  item1b
        jal  item2a
        jal  item3b
        jal  item4a
        jal  item5b
        jal  item6b
        jal  item7a
        jal  item8a
        jal  item9a
        jal  item9b
        jal  item10b
        jal  item11b
        jal  item12a
        jal  item13b

        li   a7, 10                # ecall 10 = exit the program
        ecall

# Every item starts with  mv s11, ra  and ends with  jr s11.
# jal puts the return address in ra, but the item also uses jal itself
# (to print), which overwrites ra. So the item keeps its own return
# address safe in s11. (Not in the exam.)


# ===================== Item 1 (a) =====================
# Translate this function into C. (MIPS exam, RISC-V version.)
# Answer:  int foo(int* x, int y) { int tmp = *x + y; *x = tmp; return tmp; }
item1a:
        mv   s11, ra
        la   a0, s1a
        jal  print_str
        la   a0, n                 # a0 = &n          (n = 5)
        addi a1, zero, 3           # a1 = 3
        jal  foo                   # foo(&n, 3)  -> a0 = 8, and n is now 8
        jal  print_int
        jr   s11

# The function itself:  a0 = x, a1 = y, result in a0
foo:
        lw   t0, 0(a0)             # t0 = *x
        add  t0, t0, a1            # t0 = *x + y
        sw   t0, 0(a0)             # *x = t0
        add  a0, t0, zero          # return value goes in a0
        jr   ra                    # return

# ===================== Item 1 (b) =====================
# Machine code of  sw t3,-8(a0)     (MIPS exam answer: 0xAC8BFFF8)
item1b:
        mv   s11, ra
        la   a0, buffer
        addi a0, a0, 8             # a0 = buffer + 8, so -8(a0) is buffer[0]
        li   t3, 0x1234            # a value to store
        sw   t3, -8(a0)            # <-- EXAM   0xffc52c23   (S-type)
        la   t4, buffer
        lw   t5, 0(t4)             # read it back to check
        la   a0, s1b
        jal  print_str
        mv   a0, t5
        jal  print_hex
        jr   s11

# ===================== Item 2 (a) =====================
# Which instruction is 0xFF940E13?   (MIPS exam: 0x220bfff9 = addi $t3,$s0,-7)
# Answer: addi t3,s0,-7
item2a:
        mv   s11, ra
        li   s0, 10
        addi t3, s0, -7            # <-- EXAM   0xff940e13   (I-type)   10 - 7 = 3
        la   a0, s2a
        jal  print_str
        mv   a0, t3
        jal  print_int
        jr   s11

# ===================== Item 3 (b) =====================
# t0 contains 0x10008040. Load a word from address 0x10008048 into s1, using t0.
# Offset = 0x10008048 - 0x10008040 = 8.   (MIPS exam answer: 0x8D110008)
item3b:
        mv   s11, ra
        lui  t0, 0x10008
        addi t0, t0, 0x40          # t0 = 0x10008040
        li   t4, 77
        sw   t4, 8(t0)             # put 77 at 0x10008048 first, so there is something to load
        lw   s1, 8(t0)             # <-- EXAM   0x0082a483   (I-type)
        la   a0, s3b
        jal  print_str
        mv   a0, s1
        jal  print_int
        jr   s11

# ===================== Item 4 (a) =====================
# t2 = 0x0000ff13, t3 = 0x12a30002, s0 = 0x7680bbee. Which register changes?
# Machine code 0x01C3C433 = xor s0,t2,t3   (MIPS exam: 0x014b8026)
# Answer: s0 = 0x12a3ff11
item4a:
        mv   s11, ra
        li   t2, 0x0000ff13
        li   t3, 0x12a30002
        li   s0, 0x7680bbee
        xor  s0, t2, t3            # <-- EXAM   0x01c3c433   (R-type)
        la   a0, s4a
        jal  print_str
        mv   a0, s0
        jal  print_hex
        jr   s11

# ===================== Item 5 (b) =====================
# lw s6,-11(t6) is stored in LITTLE-ENDIAN format. Which byte is at each address?
# (MIPS exam: lw $s6,-11($t8) = 0x8F16FFF5 -> f5, ff, 16, 8f.  t6 replaces $t8.)
# Little-endian: the LOWEST byte of the word is at the LOWEST address.
item5b:
        mv   s11, ra
        la   t6, buffer
        addi t6, t6, 11            # t6 = buffer + 11, so -11(t6) is buffer[0]
        lw   s6, -11(t6)           # <-- EXAM   0xff5fab03   (I-type)

        la   a0, s5b
        jal  print_str
        jal  print_newline
        la   t4, word5             # word5 holds 0xff5fab03
        lbu  a0, 0(t4)             # byte at address + 0  -> 0x03
        jal  print_hex
        lbu  a0, 1(t4)             # byte at address + 1  -> 0xab
        jal  print_hex
        lbu  a0, 2(t4)             # byte at address + 2  -> 0x5f
        jal  print_hex
        lbu  a0, 3(t4)             # byte at address + 3  -> 0xff
        jal  print_hex
        jr   s11

# ===================== Item 6 (b) =====================
# Machine code of the bne.   (MIPS exam answer: 0x1520FFFE)
# loop is 1 instruction before the bne: offset = -4 bytes.
item6b:
        mv   s11, ra
        addi t1, zero, 5
loop:   addi t1, t1, -1
        bne  t1, zero, loop        # <-- EXAM   0xfe031ee3   (B-type)
        la   a0, s6b
        jal  print_str
        mv   a0, t1
        jal  print_int
        jr   s11

# ===================== Item 7 (a) =====================
# Machine code of  addi t3,t6,-23   (MIPS exam: addi $t3,$t8,-23 = 0x230BFFE9)
item7a:
        mv   s11, ra
        li   t6, 100
        addi t3, t6, -23           # <-- EXAM   0xfe9f8e13   (I-type)   100 - 23 = 77
        la   a0, s7a
        jal  print_str
        mv   a0, t3
        jal  print_int
        jr   s11

# ===================== Item 8 (a) =====================
# t0 = 0x6, t1 = 0x7, t2 = 0xaf, s0 = 0x23, s1 = 0x3, s2 = 0x1c.
# Which register changes after machine code 0x009293b3?
# Answer: sll t2,t0,s1  ->  t2 = 6 << 3 = 48
item8a:
        mv   s11, ra
        li   t0, 0x6
        li   t1, 0x7
        li   t2, 0xaf
        li   s0, 0x23
        li   s1, 0x3
        li   s2, 0x1c
        sll  t2, t0, s1            # <-- EXAM   0x009293b3   (R-type)
        la   a0, s8a
        jal  print_str
        mv   a0, t2
        jal  print_int
        jr   s11

# ===================== Item 9 (a) =====================
# i.  Machine code of the sra?          Answer: 0x407352b3
# ii. Value of t0 afterwards?           Answer: -8
item9a:
        mv   s11, ra
        addi t1, zero, -32
        addi t2, zero, 2
        sra  t0, t1, t2            # <-- EXAM   0x407352b3   (R-type)   -32 / 4 = -8
        la   a0, s9a
        jal  print_str
        mv   a0, t0
        jal  print_int
        jr   s11

# ===================== Item 9 (b) =====================
# The two missing C lines in foo are  *p = *p + 2;  and  p++;
# bar calls foo(lst). Afterwards lst = {9, 10, 12, 0}.
item9b:
        mv   s11, ra
        la   a0, lst               # int *b = lst;  foo(b);
        jal  foo9
        la   a0, s9b
        jal  print_str
        jal  print_newline
        la   t4, lst
        lw   a0, 0(t4)
        jal  print_int
        lw   a0, 4(t4)
        jal  print_int
        lw   a0, 8(t4)
        jal  print_int
        lw   a0, 12(t4)
        jal  print_int
        jr   s11

# The function itself:  void foo(int *p)     a0 = p
foo9:
while9: lw   t0, 0(a0)             # t0 = *p
        beq  t0, zero, done9       # while (*p != 0)
        lw   t0, 0(a0)             # \
        addi t0, t0, 2             #  > code line 1:  *p = *p + 2;
        sw   t0, 0(a0)             # /
        addi a0, a0, 4             # code line 2:  p++;   (one int = 4 bytes)
        j    while9
done9:  jr   ra

# ===================== Item 10 (b) =====================
# i.  Machine code of the lui?          Answer: 0x32a152b7
# ii. Value of t0 afterwards?           Answer: 0x32a15000
item10b:
        mv   s11, ra
        addi t0, zero, 0x5ff
        lui  t0, 0x32a15           # <-- EXAM   0x32a152b7   (U-type)   replaces ALL of t0
        la   a0, s10b
        jal  print_str
        mv   a0, t0
        jal  print_hex
        jr   s11

# ===================== Item 11 (b) =====================
# Machine code of  lb t3,-11(s2)
item11b:
        mv   s11, ra
        la   s2, buffer
        addi s2, s2, 11            # s2 = buffer + 11, so -11(s2) is the first byte of buffer
        li   t4, 0xf0
        sb   t4, -11(s2)           # put the byte 0xf0 there first
        lb   t3, -11(s2)           # <-- EXAM   0xff590e03   (I-type)   lb sign-extends: 0xf0 -> -16
        la   a0, s11b
        jal  print_str
        mv   a0, t3
        jal  print_int
        jr   s11

# ===================== Item 12 (a) =====================
# Which instruction is 0xff35ac23?     Answer: sw s3,-8(a1)
item12a:
        mv   s11, ra
        la   a1, buffer
        addi a1, a1, 12            # a1 = buffer + 12, so -8(a1) is buffer[1]
        li   s3, 99
        sw   s3, -8(a1)            # <-- EXAM   0xff35ac23   (S-type)
        la   t4, buffer
        lw   t5, 4(t4)             # read buffer[1] back to check
        la   a0, s12a
        jal  print_str
        mv   a0, t5
        jal  print_int
        jr   s11

# ===================== Item 13 (b) =====================
# Machine code of the bne.
# repeat is 2 instructions before the bne: offset = -8 bytes.
item13b:
        mv   s11, ra
        li   s3, 0                 # (not in the exam) start the counter at 0
        li   s1, 1                 # (not in the exam) count 1 per round
        addi t3, zero, 220
repeat: addi t3, t3, -2
        add  s3, s3, s1
        bne  t3, zero, repeat      # <-- EXAM   0xfe0e1ce3   (B-type)
        la   a0, s13b
        jal  print_str
        mv   a0, s3
        jal  print_int
        jr   s11


# =====================================================================
# Print helpers (not in the exam)
# =====================================================================

print_str:                         # print the text that a0 points at
        li   a7, 4
        ecall
        jr   ra

print_int:                         # print a0 as a decimal number, then a new line
        li   a7, 1
        ecall
        j    print_newline

print_hex:                         # print a0 as a hex number, then a new line
        li   a7, 34
        ecall
        j    print_newline

print_newline:
        li   a0, 10                # 10 = the new line character
        li   a7, 11
        ecall
        jr   ra
