# =====================================================================
# Module 2, Q2: I/O SYSTEMS - the ASSEMBLY "code" questions
#
# Every exam item is its own function (item1a, item2b, ...).
# main calls them one after another. To run only some items, put a #
# in front of the calls you do not want.
#
# THE PORTS ARE FAKE HERE.
# On the exam a port (LEDs, switches, buttons) is a word of memory at a
# fixed address such as 0xb8004700. A simulator has no memory there, so
# the program would crash. In this file a word in .data plays the role
# of each port. That changes ONLY the lines that put the address in a
# register:
#
#     exam:       lui  t0, 0xb8004        here:   la   t0, port8a
#                 ori  t0, t0, 0x700
#
# Every other instruction is the same. Above each item there is a block
# "WRITE THIS ON THE EXAM" with the real address lines. The lines that
# differ in the runnable code are marked  <-- fake port.
# Some ports sit a little way into a block of memory (for example
# "io4b" with the switches at +0x20 and the LEDs at +0x40). Then the
# exam's  lw t1,0x20(t0)  can be used unchanged.
#
# HOW TO PUT A 32-BIT ADDRESS IN A REGISTER (worth points in every item):
#     lui  t0, <upper 20 bits>        the 5 first hex digits
#     addi t0, t0, <lower 12 bits>    the 3 last hex digits   (or ori)
#   0x95003700  ->  lui t0,0x95003  +  0x700
#   BUT the 12-bit number of addi/ori/andi is SIGNED: -2048 ... 2047.
#   If the 3 last hex digits are 0x800 or more they count as negative.
#   Then use one more in the lui and a negative addi:
#   0xb8004d00  ->  lui t0,0xb8005  +  addi t0,t0,-768    (0xd00 - 0x1000 = -0x300 = -768)
#   The function addr_demo below prints these, so you can check them.
#   If the 3 last digits are 000, lui alone is enough.
#   If they are small you can also keep them as the offset: lw t1,0x20(t0).
#
# THE THREE MOVES:
#   turn a bit ON:    ori  t1, t1, <1 at that bit>
#   turn a bit OFF:   andi t1, t1, <0 at that bit, 1 everywhere else>
#                     (write it as a negative number: -9 = 0xfffffff7 clears bit 3)
#   keep the other bits: lw the port first, change, then sw it back.
#
# "NO PSEUDO INSTRUCTIONS": li, la and mv are pseudo instructions. So are
# j and jr in RISC-V (the real ones are  jal zero,label  and
# jalr zero,0(ra)), but the examiner's own answers use j and jr.
#
# Items 1 to 7 and the two extra items come from old MIPS exams. Their
# RISC-V code is THIS FILE'S OWN translation, not the examiner's. The
# examiner's MIPS answer is shown for reference.
#
# Run in RARS (a RISC-V simulator):
#   1. Open this file in RARS.   2. Assemble (F3).   3. Run (F5).
# From the terminal:  java -jar rars1_6.jar module2_io.asm
# =====================================================================

.data
# ----- fake ports (one set for each item, so the items do not disturb each other) -----
port1a:  .word 0xa5                # item 1(a): LED port, exam address 0xccbb0000. 1010 0101, LED 3 off

io2b:    .space 0x20               # item 2(b): this block plays the role of 0xabab0000 ...
         .word 0x1234              #   ... and the port is at +0x20 (exam address 0xabab0020)

io3a:    .space 0x44               # item 3(a): block for 0xffff0000 ...
         .word 0xabcd1234          #   ... port at +0x44 (exam address 0xffff0044)

io4b:    .space 0x20               # item 4(b): block for 0xabab0000 ...
         .word 0x90                #   ... switches at +0x20: switch 8 on, switch 5 on
         .space 0x1c
         .word 0                   #   ... LEDs at +0x40

btn5:    .word 0x20                # item 5(a): button port, exam address 0xffff0230. Bit 5 = 1: ALREADY PUSHED
led5:    .word 0x81                # item 5(a): LED port, exam address 0xffffaa10

port6a:  .word 0xff3f              # item 6(a): exam address 0xb8004d00. LEDs 1,2 off, LEDs 3,4 on, the rest 1

sw7:     .word 0x80                # item 7(b): switches, exam address 0x1b430010. Bits 7..6 = 10
led7:    .word 0xffff              # item 7(b): LEDs, exam address 0xabbb8050. All 16 on

port8a:  .word 0                   # item 8(a): exam address 0xb8004700

port9b:  .word 0xffffffff          # item 9(b): exam address 0x95003700. Every bit 1 to start with

io10:    .space 0x40               # item 10(a): block for 0x10010000 ...
         .word 1                   #   ... push buttons at +0x40: button 1 pushed
         .space 0xc
         .word 0xa50               #   ... switches at +0x50: switches 1 and 3 up

io11:    .space 0xd0               # item 11(a): block for 0xbf886000 ...
         .word 0xd00               #   ... switches at +0xd0: bits 11..8 = 1101 (switch 4 on, value 5)
         .space 0x3c
         .word 0xab00              #   ... LEDs at +0x110

btn12:   .word 0                   # item 12(b): buttons, exam address 0xffffe440
led12:   .word 0                   # item 12(b): LEDs, exam address 0x17880010
seq12:   .word 0x00, 0x80, 0x00, 0x10, 0x80, 0x90     # the buttons the "user" presses, round by round

led13:   .word 0xfe7               # item 13(b): LEDs, exam address 0x1dbb8050
sw13:    .word 0x20                # item 13(b): switches, exam address 0x22430030. Bits 5..4 = 10

portx15: .word 0xa5                # extra Q2.15: exam address 0xbf226000. LED #4 off
portx16: .word 0                   # extra Q2.16: exam address 0x800020f0

sad1: .string "Address demo: lui 0xb8005, addi -768  (should be 0xb8004d00): "
sad2: .string "Address demo: lui 0xffffb, addi -1520 (should be 0xffffaa10): "
sad3: .string "Address demo: lui 0x10, addi -97      (should be 0x0000ff9f): "
s1a:  .string "Item 1(a)  port 0xa5, led3(1) then led3(0) (key 0x000000ad, 0x000000a5):"
s2b:  .string "Item 2(b)  get_status() with port 0x1234 returns (key 1): "
s3a:  .string "Item 3(a)  readval() with port 0xabcd1234 returns (key 0x00000023): "
s4b:  .string "Item 4(b)  switches 0x90: LEDs after the loop (key 1): "
s5a:  .string "Item 5(a)  check_button: LED port 0x81 becomes (key 0x00000085): "
s6a:  .string "Item 6(a)  port 0x0000ff3f becomes (key 0x0000fcff): "
s7b:  .string "Item 7(b)  switches 0x80, LEDs 0xffff: LEDs become (key 0x0000ffdf): "
s8a:  .string "Item 8(a)  twinkle(6): port after each step (key 0x100, 0, 0x100, 0, 0x100, 0):"
s8a2: .string "           returned sum of the pauses (key 60): "
s9b:  .string "Item 9(b)  port 0xffffffff becomes (key 0xfffffe7f): "
s10a: .string "Item 10(a) getvals() with switches 0xa50 returns (key 5): "
s11a: .string "Item 11(a) LEDs for switches 1101 then 0101 (key 0x0000ab19, 0x0000ab0a):"
s12b: .string "Item 12(b) LEDs for buttons -, A, -, D, A, A+D (key 0, 0x1a0, 0x1a0, 0, 0x1a0, 0):"
s13b: .string "Item 13(b) LEDs 0xfe7, switches 0x20: LEDs become (key 0x00000797): "
sx15: .string "extra Q2.15  port 0xa5, led4(0xffffffff) then led4(0xfffffffe) (key 0x000000ad, 0x000000a5):"
sx16: .string "extra Q2.16  digit 6: port becomes (key 0x0000007c): "

.text

# =====================================================================
# main: put a # in front of a call to skip that item
# =====================================================================
main:
        jal  addr_demo
        jal  item1a
        jal  item2b
        jal  item3a
        jal  item4b
        jal  item5a
        jal  item6a
        jal  item7b
        jal  item8a
        jal  item9b
        jal  item10a
        jal  item11a
        jal  item12b
        jal  item13b
        jal  extra_Q2_15
        jal  extra_Q2_16

        li   a7, 10                # ecall 10 = exit the program
        ecall

# Every item starts with  mv s11, ra  and ends with  jr s11.
# jal puts the return address in ra, but the item also uses jal itself
# (to print), which overwrites ra. So the item keeps its own return
# address safe in s11. (Not in the exam.)


# ===================== Address demo (not an exam item) =====================
# Builds the three "difficult" constants used in this file and prints
# them, to show that  lui (one more) + negative addi  gives the right number.
addr_demo:
        mv   s11, ra
        la   a0, sad1
        jal  print_str
        lui  t0, 0xb8005           # t0 = 0xb8005000
        addi t0, t0, -768          # t0 = 0xb8005000 - 0x300 = 0xb8004d00
        mv   a0, t0
        jal  print_hex
        la   a0, sad2
        jal  print_str
        lui  t1, 0xffffb           # t1 = 0xffffb000
        addi t1, t1, -1520         # t1 = 0xffffb000 - 0x5f0 = 0xffffaa10
        mv   a0, t1
        jal  print_hex
        la   a0, sad3
        jal  print_str
        lui  t4, 0x10              # t4 = 0x00010000
        addi t4, t4, -97           # t4 = 0x00010000 - 0x61 = 0x0000ff9f
        mv   a0, t4
        jal  print_hex
        jr   s11


# ===================== Item 1 (a) =====================
# 2016-06-09 Q2.pdf   (same as Q2.13fill.png)          ORIGINAL WAS MIPS
# 8 LEDs on the port at 0xccbb0000, LED 0 = bit 0 ... LED 7 = bit 7.
# Complete the function led3: argument a0 = 1 turns LED 3 on, a0 = 0
# turns it off. No other LED may change. No extra labels, no pseudo
# instructions. Template:   led3: (one instruction) / lw t0,0(t1) / (several)
#
# Examiner's MIPS answer:
#     led3: lui  $t1,0xccbb
#           lw   $t0,0($t1)
#           sll  $a0,$a0,3
#           andi $t0,$t0,0xfff7
#           or   $t0,$t0,$a0
#           sw   $t0,0($t1)
#           jr   $ra
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     led3: lui  t1, 0xccbb0         # t1 = 0xccbb0000 (the last 3 hex digits are 000)
#           lw   t0, 0(t1)           # read the port
#           slli a0, a0, 3           # move the argument to bit 3
#           andi t0, t0, -9          # clear bit 3   (-9 = 0xfffffff7)
#           or   t0, t0, a0          # put the new bit 3 in
#           sw   t0, 0(t1)           # write the port
#           jr   ra
# (MIPS andi 0xfff7 also clears bits 31..16. RISC-V andi -9 clears only bit 3.)
item1a:
        mv   s11, ra
        la   a0, s1a
        jal  print_str
        jal  print_newline
        addi a0, zero, 1
        jal  led3                  # led3(1): LED 3 on    0xa5 -> 0xad
        la   t4, port1a
        lw   a0, 0(t4)
        jal  print_hex
        addi a0, zero, 0
        jal  led3                  # led3(0): LED 3 off   0xad -> 0xa5
        la   t4, port1a
        lw   a0, 0(t4)
        jal  print_hex
        jr   s11

led3:
        la   t1, port1a            # <-- fake port.  exam: lui t1, 0xccbb0
        lw   t0, 0(t1)             # read the port
        slli a0, a0, 3             # move the argument (0 or 1) to bit 3
        andi t0, t0, -9            # clear bit 3, keep all other bits
        or   t0, t0, a0            # put the new bit 3 in
        sw   t0, 0(t1)             # write the port
        jr   ra

# ===================== Item 2 (b) =====================
# 2017-01-12 Q2.pdf   (same as Q2.12translate.png)     ORIGINAL WAS MIPS
# Translate to assembly, with function label and calling convention:
#     int get_status(){
#       volatile int* toggle = (volatile int*) 0xabab0020;
#       return (*toggle & 0x4) >> 2;
#     }
# (The C version runs in module2_io.c, item2b.)
#
# Examiner's MIPS answer:
#     get_status: lui  $t0,0xabab
#                 lw   $v0,0x20($t0)
#                 andi $v0,$v0,0x4
#                 srl  $v0,$v0,2
#                 jr   $ra
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     get_status: lui  t0, 0xabab0       # t0 = 0xabab0000
#                 lw   a0, 0x20(t0)      # a0 = *toggle   (address 0xabab0020)
#                 andi a0, a0, 0x4       # keep bit 2
#                 srli a0, a0, 2         # move it down to bit 0
#                 jr   ra                # the return value is in a0
item2b:
        mv   s11, ra
        la   a0, s2b
        jal  print_str
        jal  get_status
        jal  print_int
        jr   s11

get_status:
        la   t0, io2b              # <-- fake port.  exam: lui t0, 0xabab0
        lw   a0, 0x20(t0)          # a0 = *toggle
        andi a0, a0, 0x4           # keep bit 2
        srli a0, a0, 2             # move it down to bit 0
        jr   ra                    # the return value is in a0

# ===================== Item 3 (a) =====================
# 2017-03-16 Q2.pdf   (same as Q2.11create.png)        ORIGINAL WAS MIPS
# Input port at 0xffff0044. Write a function readval that reads the
# port and returns bits 11..4 as the lowest 8 bits. The other 24 bits
# of the result must be 0. Follow the calling convention, no pseudo
# instructions.
#
# Examiner's MIPS answer:
#     readval: lui  $t0,0xffff
#              lw   $v0,0x44($t0)
#              srl  $v0,$v0,4
#              andi $v0,$v0,0xff
#              jr   $ra
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     readval: lui  t0, 0xffff0          # t0 = 0xffff0000
#              lw   a0, 0x44(t0)         # read the port (address 0xffff0044)
#              srli a0, a0, 4            # bits 11..4 move down to bits 7..0
#              andi a0, a0, 0xff         # keep 8 bits, the rest become 0
#              jr   ra
# (The key takes away a point if you use an s register without saving it
#  on the stack. Use t registers and a0.)
item3a:
        mv   s11, ra
        la   a0, s3a
        jal  print_str
        jal  readval
        jal  print_hex
        jr   s11

readval:
        la   t0, io3a              # <-- fake port.  exam: lui t0, 0xffff0
        lw   a0, 0x44(t0)          # read the port:  0xabcd1234
        srli a0, a0, 4             # bits 11..4 move down to bits 7..0:  0x0abcd123
        andi a0, a0, 0xff          # keep 8 bits:  0x23
        jr   ra

# ===================== Item 4 (b) =====================
# 2017-06-08 Q2.pdf   (same as Q2.10filltranslate.png)  ORIGINAL WAS MIPS
# Translate to assembly, without pseudo instructions:
#     volatile int* switches = (volatile int*) 0xabab0020;
#     volatile int* leds = (volatile int*) 0xabab0040;
#     while(*switches & 0x80){            <- the answer of part (a)
#       *leds = (*switches >> 4) & 1;
#     }
# (Part (a) and the C version are in module2_io.c, item4a.)
#
# Examiner's MIPS answer:
#           lui  $t0,0xabab
#     loop: lw   $t1,0x20($t0)
#           andi $t2,$t1,0x80
#           beq  $t2,$zero,exit
#           srl  $t1,$t1,4
#           andi $t1,$t1,1
#           sw   $t1,0x40($t0)
#           j    loop
#     exit:
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#           lui  t0, 0xabab0             # t0 = 0xabab0000
#     loop: lw   t1, 0x20(t0)            # t1 = *switches
#           andi t2, t1, 0x80            # t2 = *switches & 0x80   (switch 8)
#           beq  t2, zero, exit          # switch 8 off -> leave the loop
#           srli t1, t1, 4               # *switches >> 4
#           andi t1, t1, 1               # & 1
#           sw   t1, 0x40(t0)            # *leds = ...
#           j    loop
#     exit:
#
# THE LOOP MUST END HERE: on the exam the loop runs until somebody
# switches switch 8 off. Nobody can do that in a simulator, so three
# marked lines are added that switch it off after the first round.
item4b:
        mv   s11, ra
        la   t0, io4b              # <-- fake port.  exam: lui t0, 0xabab0
loop4:  lw   t1, 0x20(t0)          # t1 = *switches            (0x90 the first time)
        andi t2, t1, 0x80          # t2 = *switches & 0x80
        beq  t2, zero, exit4       # switch 8 off -> leave the loop
        srli t1, t1, 4             # *switches >> 4
        andi t1, t1, 1             # & 1   -> 1, because switch 5 (bit 4) is on
        sw   t1, 0x40(t0)          # *leds = 1
        # ---------- ADDED (not in the exam): the "user" switches switch 8 off ----------
        lw   t3, 0x20(t0)
        andi t3, t3, -129          # clear bit 7   (-129 = 0xffffff7f)
        sw   t3, 0x20(t0)
        # -----------------------------------------------------------------------------------
        j    loop4
exit4:
        la   a0, s4b
        jal  print_str
        la   t0, io4b
        lw   a0, 0x40(t0)          # read the LEDs back to check
        jal  print_int
        jr   s11

# ===================== Item 5 (a) =====================
# 2018-01-15 Q2.pdf   (same as Q2.9create.png)         ORIGINAL WAS MIPS
# Write a function check_button. It checks a push button again and
# again. The button is bit 5 of the port at 0xffff0230 (1 = pushed).
# When it is pushed: turn on the LED at bit 2 of the port at 0xffffaa10
# without changing any other bit of that port, then return.
#
# Examiner's MIPS answer:
#     check_button: lui  $t0,0xffff
#                   ori  $t1,$t0,0xaa10
#                   ori  $t0,$t0,0x0230
#     loop:         lw   $t2,0($t0)
#                   andi $t2,$t2,32
#                   beq  $t2,$zero,loop
#                   lw   $t2,0($t1)
#                   ori  $t2,$t2,4
#                   sw   $t2,0($t1)
#                   jr   $ra
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     check_button: lui  t0, 0xffff0
#                   addi t0, t0, 0x230       # t0 = 0xffff0230 (buttons)
#                   lui  t1, 0xffffb         # 0xa10 is 0x800 or more: one more in the lui ...
#                   addi t1, t1, -1520       # ... and 0xa10 - 0x1000 = -1520.  t1 = 0xffffaa10 (LED)
#     loop:         lw   t2, 0(t0)           # read the buttons
#                   andi t2, t2, 32          # keep bit 5   (32 = 0x20)
#                   beq  t2, zero, loop      # not pushed -> check again   (this is POLLING)
#                   lw   t2, 0(t1)           # read the LED port
#                   ori  t2, t2, 4           # turn on bit 2, keep the rest
#                   sw   t2, 0(t1)           # write it back
#                   jr   ra
#
# THE LOOP MUST END HERE: the fake button port (btn5 in .data) already
# holds 0x20, so the button is "pushed" from the start and the loop
# exits on its first check.
item5a:
        mv   s11, ra
        jal  check_button
        la   a0, s5a
        jal  print_str
        la   t4, led5
        lw   a0, 0(t4)             # read the LED port back to check
        jal  print_hex
        jr   s11

check_button:
        la   t0, btn5              # <-- fake port.  exam: lui t0,0xffff0 / addi t0,t0,0x230
        la   t1, led5              # <-- fake port.  exam: lui t1,0xffffb / addi t1,t1,-1520
loop5:  lw   t2, 0(t0)             # read the buttons
        andi t2, t2, 32            # keep bit 5
        beq  t2, zero, loop5       # not pushed -> check again
        lw   t2, 0(t1)             # read the LED port:   0x81
        ori  t2, t2, 4             # turn on bit 2:       0x85
        sw   t2, 0(t1)             # write it back
        jr   ra

# ===================== Item 6 (a) =====================
# 2018-03-15 Q2.pdf   (same as Q2.8create.png)         ORIGINAL WAS MIPS
# Four LEDs on the port at 0xb8004d00: LED 1 = bit 6, LED 2 = bit 7,
# LED 3 = bit 8, LED 4 = bit 9. Turn ON LEDs 1 and 2 and turn OFF LEDs
# 3 and 4. No pseudo instructions. None of the other 16 lowest bits may
# change.
#
# Examiner's MIPS answer:
#     lui  $t0,0xb800
#     ori  $t0,$t0,0x4d00
#     lw   $t1,0($t0)
#     ori  $t1,$t1,0xc0
#     andi $t1,$t1,0xfcff
#     sw   $t1,0($t0)
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     lui  t0, 0xb8005             # 0xd00 is 0x800 or more: one more in the lui ...
#     addi t0, t0, -768            # ... and 0xd00 - 0x1000 = -768.   t0 = 0xb8004d00
#     lw   t1, 0(t0)               # read the port
#     ori  t1, t1, 0xc0            # 1100 0000: turn on bits 7 and 6 (LEDs 2 and 1)
#     andi t1, t1, -769            # -769 = 0xfffffcff: turn off bits 9 and 8 (LEDs 4 and 3)
#     sw   t1, 0(t0)               # write it back
item6a:
        mv   s11, ra
        la   t0, port6a            # <-- fake port.  exam: lui t0,0xb8005 / addi t0,t0,-768
        lw   t1, 0(t0)             # read the port:              0x0000ff3f
        ori  t1, t1, 0xc0          # turn on bits 7 and 6:       0x0000ffff
        andi t1, t1, -769          # turn off bits 9 and 8:      0x0000fcff
        sw   t1, 0(t0)             # write it back
        la   a0, s6a
        jal  print_str
        la   t0, port6a
        lw   a0, 0(t0)             # read it back to check
        jal  print_hex
        jr   s11

# ===================== Item 7 (b) =====================
# 2018-06-05 Q2.pdf   (same as Q2.7filltranslate.png)  ORIGINAL WAS MIPS
# Translate to assembly, without pseudo instructions:
#     volatile int* switches = (volatile int*) 0x1b430010;
#     volatile int* leds = (volatile int*) 0xabbb8050;
#     int temp = ((*switches) >> 6) & 0x3;
#     *leds = (*leds & 0xff9f) | (temp << 5);     <- temp << 5 is the answer of part (a)
# (Part (a) and the C version are in module2_io.c, item7a.)
#
# Examiner's MIPS answer:
#     lui  $t1,0x1b43
#     ori  $t1,$t1,0x0010
#     lui  $t2,0xabbb
#     ori  $t2,$t2,0x8050
#     lw   $t0,0($t1)
#     srl  $t0,$t0,6
#     andi $t0,$t0,0x3
#     lw   $t3,0($t2)
#     andi $t3,$t3,0xff9f
#     sll  $t0,$t0,5
#     or   $t3,$t3,$t0
#     sw   $t3,0($t2)
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     lui  t1, 0x1b430
#     addi t1, t1, 0x10            # t1 = 0x1b430010 (switches)
#     lui  t2, 0xabbb8
#     addi t2, t2, 0x50            # t2 = 0xabbb8050 (leds)
#     lw   t0, 0(t1)               # t0 = *switches
#     srli t0, t0, 6               # >> 6
#     andi t0, t0, 0x3             # & 0x3            t0 is temp
#     lw   t3, 0(t2)               # t3 = *leds
#     lui  t4, 0x10                # 0xff9f does not fit in andi's 12 bits (and -97
#     addi t4, t4, -97             #   would be 0xffffff9f), so build 0x0000ff9f in t4
#     and  t3, t3, t4              # *leds & 0xff9f
#     slli t0, t0, 5               # temp << 5
#     or   t3, t3, t0              # |
#     sw   t3, 0(t2)               # *leds = ...
item7b:
        mv   s11, ra
        la   t1, sw7               # <-- fake port.  exam: lui t1,0x1b430 / addi t1,t1,0x10
        la   t2, led7              # <-- fake port.  exam: lui t2,0xabbb8 / addi t2,t2,0x50
        lw   t0, 0(t1)             # t0 = *switches            0x80
        srli t0, t0, 6             # >> 6                      2
        andi t0, t0, 0x3           # & 0x3                     temp = 2
        lw   t3, 0(t2)             # t3 = *leds                0xffff
        lui  t4, 0x10              # t4 = 0x00010000
        addi t4, t4, -97           # t4 = 0x0000ff9f
        and  t3, t3, t4            # *leds & 0xff9f            0xff9f
        slli t0, t0, 5             # temp << 5                 0x40
        or   t3, t3, t0            # |                         0xffdf
        sw   t3, 0(t2)             # *leds = 0xffdf
        la   a0, s7b
        jal  print_str
        la   t2, led7
        lw   a0, 0(t2)             # read it back to check
        jal  print_hex
        jr   s11

# ===================== Item 8 (a) =====================
# 2019-01-12-riscv Q2.pdf   (same as Q2.1code.png and Q2.6create.png)
# Four LEDs on the port at 0xb8004700: LED 1 = bit 6 ... LED 4 = bit 9.
# The other bits of the port need not be kept.
# Write a function twinkle that can be called from C. Its argument
# (unsigned) says how many steps to do. Each step turns LED 3 on or off
# (on the first time), flipping the bit with xori. All other LEDs are
# off. After every step it calls pause, which returns how many
# milliseconds it paused and does not destroy t registers. twinkle
# returns the sum of all the pauses. No pseudo instructions, no stack.
#
# WRITE THIS ON THE EXAM (the examiner's answer):
#     twinkle:
#       lui    t0,0xb8004            # Setup port address
#       ori    t0,t0,0x700
#       add    t1,zero,ra            # Save the return address
#       addi   t2,zero,0x100         # Initiate 1 to LED 3 (index 8)
#       addi   t3,zero,0x0           # Set sum up variable to zero
#     loop:
#       sw     t2,0(t0)              # Turn on / off the LED 3
#       xori   t2,t2,0x100           # Flip the bit
#       add    t4, zero, a0          # store the content of a0
#       jal    pause                 # Call pause
#       add    t3,t3,a0              # Add together time values
#       add    a0, zero, t4          # restore the content of a0
#       addi   a0,a0,-1
#       bne    a0,zero,loop          # Repeat
#       addi   a0,t3,0               # Set return value
#       jr     t1                    # Return using saved address
#   "The function only needs to behave well for input values larger than zero."
#
# Why ra is saved in t1: jal pause overwrites ra, and the stack is not
# allowed. pause promises not to touch t registers, so t1 is safe.
# Why a0 is saved in t4: pause returns its result in a0, which would
# destroy the counter.
item8a:
        mv   s11, ra
        la   a0, s8a
        jal  print_str
        jal  print_newline
        addi a0, zero, 6
        jal  twinkle               # twinkle(6); pause prints the port after each step
        mv   s0, a0                # keep the returned sum
        la   a0, s8a2
        jal  print_str
        mv   a0, s0
        jal  print_int
        jr   s11

twinkle:
        la   t0, port8a            # <-- fake port.  exam: lui t0,0xb8004 / ori t0,t0,0x700
        add  t1, zero, ra          # save the return address
        addi t2, zero, 0x100       # bit 8 = LED 3. Start with "on"
        addi t3, zero, 0x0         # sum = 0
loop8:
        sw   t2, 0(t0)             # turn LED 3 on / off (all other bits 0)
        xori t2, t2, 0x100         # flip bit 8 for the next step
        add  t4, zero, a0          # keep the counter safe in t4
        jal  pause                 # a0 = milliseconds paused
        add  t3, t3, a0            # sum = sum + milliseconds
        add  a0, zero, t4          # get the counter back
        addi a0, a0, -1            # one step done
        bne  a0, zero, loop8       # more steps left -> repeat
        addi a0, t3, 0             # return value = sum
        jr   t1                    # return with the saved address

# pause is NOT in the exam ("you can assume that this function exists").
# This one prints the port, so that every step can be seen, and says it
# paused for 10 milliseconds. It uses no t registers.
pause:
        mv   s10, ra
        la   a0, port8a
        lw   a0, 0(a0)
        jal  print_hex
        addi a0, zero, 10          # return 10 (milliseconds)
        jr   s10

# ===================== Item 9 (b) =====================
# 2019-03-15-riscv Q2.pdf   (same as Q2.2code.png and Q2.5create.png)
# A 7-segment display is on the port at 0x95003700. Segments 0..6 are
# bits 3..9 (segment 0 = bit 3 ... segment 6 = bit 9).
#
#        0
#      -----
#   5 |     | 1          Show the digit 3: segments 5 and 4 off,
#     |  6  |            all the others on. No other bit of the port
#      -----             may change, so the port must be read first.
#   4 |     | 2          No pseudo instructions.
#     |     |
#      -----
#        3
#
# WRITE THIS ON THE EXAM (the examiner's answer):
#     lui     t0,0x95003           # Get the correct address into t0
#     ori     t0,t0,0x700
#     lw      t1,0(t0)             # Load the current value of the port
#     li      t2,0xfffffc07        # Mask the 7 LED bits
#     and     t1,t1,t2
#     ori     t1,t1,0x278          # Or on the bits that represent value 3
#     sw      t1,0(t0)             # Store the new value
#
# The numbers: bits 9..3 must become  1 0 0 1 1 1 1  (segments 6 5 4 3 2 1 0).
#   0x278      = 10 0111 1000  = that pattern, placed at bits 9..3.
#   0xfffffc07 = ...1100 0000 0111 = 0 at bits 9..3, 1 everywhere else.
# NOTE: li is a pseudo instruction, although the question forbids them
# and the examiner uses it anyway. Without a pseudo instruction the two
# lines  li / and  can be the single line   andi t1,t1,-1017
# (-1017 = 0xfffffc07, and it fits in 12 bits).
item9b:
        mv   s11, ra
        la   t0, port9b            # <-- fake port.  exam: lui t0,0x95003 / ori t0,t0,0x700
        lw   t1, 0(t0)             # read the port:                        0xffffffff
        li   t2, 0xfffffc07        # mask: 0 at the 7 segment bits
        and  t1, t1, t2            # all 7 segments off, the rest kept:    0xfffffc07
        ori  t1, t1, 0x278         # segments 6, 3, 2, 1, 0 on:            0xfffffe7f
        sw   t1, 0(t0)             # write it back
        la   a0, s9b
        jal  print_str
        la   t0, port9b
        lw   a0, 0(t0)             # read it back to check
        jal  print_hex
        jr   s11

# ===================== Item 10 (a) ii =====================
# 2019-06-05-riscv Q2.pdf   (same as Q2.3code.png and Q2.4translate.png)
# Write the complete assembly code for this C function:
#     int getvals(){
#       volatile int *pushbutton = (volatile int*) 0x10010040;
#       volatile int *switches = (volatile int*) 0x10010050;
#       int v=0;
#       while(*pushbutton & 1){
#         v = (*switches >> 4) & 0xf;
#       }
#       return v;
#     }
# (Parts i, iii, iv and the C version are in module2_io.c, item10a.)
#
# WRITE THIS ON THE EXAM (the examiner's answer, "one possible solution"):
#     getvals:
#             addi    a0,zero,0
#             lui     t1,0x10010
#     while:
#             lw      t0,0x40(t1)
#             andi    t0,t0,1
#             beq     t0,zero,exit
#             lw      a0,0x50(t1)
#             srli    a0,a0,4
#             andi    a0,a0,0xf
#             j       while
#     exit:
#             jr      ra
#
# THE LOOP MUST END HERE: on the exam the loop runs until button 1 is
# released. One marked line is added that "releases" the button after
# the first round.
item10a:
        mv   s11, ra
        la   a0, s10a
        jal  print_str
        jal  getvals
        jal  print_int
        jr   s11

getvals:
        addi a0, zero, 0           # v = 0   (v lives in a0, the return register)
        la   t1, io10              # <-- fake port.  exam: lui t1,0x10010
while10:
        lw   t0, 0x40(t1)          # t0 = *pushbutton
        andi t0, t0, 1             # & 1   (button 1)
        beq  t0, zero, exit10      # button 1 not pushed -> leave the loop
        lw   a0, 0x50(t1)          # a0 = *switches            0xa50
        srli a0, a0, 4             # >> 4                      0xa5
        andi a0, a0, 0xf           # & 0xf                     5
        sw   zero, 0x40(t1)        # ADDED (not in the exam): the "user" releases the buttons
        j    while10
exit10:
        jr   ra                    # return v

# ===================== Item 11 (a) =====================
# 2020-01-13-riscv Q2.pdf   (same as Q2.4code.png and Q2.3fill.png)
# A pocket calculator. Switches are bits 11..8 of the word at 0xbf8860d0
# (switch 4 = bit 11, switches 3..1 = bits 10..8 = a 3-bit value).
# Green LEDs are bits 7..0 of the word at 0xbf886110.
#   switch 4 = 1: show value * value.     switch 4 = 0: show value + value.
# Fill in PART I (get the switches, branch, square) and PART II (write
# the result so that only the 8 LED bits change; the upper 16 bits need
# not be kept). Template:
#     loop:    lui  t0,0xbf886
#              lw   t1,0x0D0(t0)
#              -- MISSING CODE: PART I --
#     do_add:  add  t1,t2,t2
#     next:    -- MISSING CODE: PART II --
#              j    loop
#
# WRITE THIS ON THE EXAM (the examiner's answer):
#     loop:
#             lui     t0,0xbf886
#             lw      t1,0x0d0(t0)
#             srli    t1,t1,8              # PART 1 starts
#             andi    t2,t1,0x7
#             andi    t3,t1,0x8
#             beq     t3,zero,do_add
#             mul     t1,t2,t2
#             j       next                 # PART 1 ends
#     do_add:
#             add     t1,t2,t2
#     next:
#             lw      t2,0x110(t0)         # PART 2 starts
#             li      t4, 0xffffff00
#             and     t2,t2,t4
#             or      t1,t2,t1
#             sw      t1,0x110(t0)         # PART 2 ends
#             j       loop
#
# THE LOOP MUST END HERE: the exam's loop is endless (j loop). Here the
# last line is  jr ra  instead, so that one call does one round. The
# item calls it twice, with switch 4 on and then with switch 4 off.
item11a:
        mv   s11, ra
        la   a0, s11a
        jal  print_str
        jal  print_newline
        jal  calc11                # switches 1101: switch 4 on, value 5 -> 5 * 5 = 25 = 0x19
        la   t0, io11
        lw   a0, 0x110(t0)
        jal  print_hex
        la   t0, io11
        addi t5, zero, 0x500       # the "user" turns switch 4 off: switches 0101
        sw   t5, 0xd0(t0)
        jal  calc11                # switch 4 off, value 5 -> 5 + 5 = 10 = 0x0a
        la   t0, io11
        lw   a0, 0x110(t0)
        jal  print_hex
        jr   s11

calc11:
        la   t0, io11              # <-- fake port.  exam: lui t0,0xbf886
        lw   t1, 0x0d0(t0)         # read the switches
        srli t1, t1, 8             # PART I: bits 11..8 move down to bits 3..0
        andi t2, t1, 0x7           #   t2 = the 3-bit value (switches 3..1)
        andi t3, t1, 0x8           #   t3 = switch 4 (now bit 3)
        beq  t3, zero, do_add      #   switch 4 off -> go and double
        mul  t1, t2, t2            #   switch 4 on: t1 = value * value
        j    next11                #   jump over the doubling
do_add:
        add  t1, t2, t2            # (template) t1 = value + value
next11:
        lw   t2, 0x110(t0)         # PART II: read the LED port
        li   t4, 0xffffff00        #   mask: 0 at the 8 LED bits
        and  t2, t2, t4            #   LED bits off, the rest kept
        or   t1, t2, t1            #   put the result in the LED bits
        sw   t1, 0x110(t0)         #   write the LED port
        jr   ra                    # <-- exam: j loop

# ===================== Item 12 (b) =====================
# 2020-03-11-riscv Q2.pdf   (same as Q2.5code.png and Q2.2create.png)
# Push buttons A, B, C, D are bits 7, 6, 5, 4 of the port at 0xffffe440
# (1 = pushed). 8 LEDs are bits 12..5 of the port at 0x17880010. The
# other bits of the LED port need not be kept.
# Write a program that loops forever: if A is pushed, show 13 (binary)
# on the LEDs. Keep showing 13 after A is released, until D is pushed:
# then all LEDs off. A and D together: off. At the start: off.
#
# The examiner's answer, exactly as printed:
#         lui     t0,0x17880
#         ori     t0, 0x010            # setup address to LED lights
#         addi    t3,zero,0
#         lui     t4, 0xffffe
#         ori     t4, 0x440            # setup address to buttons lights
#     loop:
#         lw      t1,0(t4)             # get the status value of the buttons
#         andi    t2,t1,0x80           # clear bits, execpt bit for button A
#         beq     t2,0,notA
#         addi    t3,zero,13           # value 13, because button A was pushed
#     notA:
#         andi    t2,t1,0x10           # clear all, execpt the bit for button D
#         beq     t2,zero,notD
#         addi    t3,zero,0            # save value 0, i.e., turn off all LEDS
#     notD:
#         slli    t3,t3,5              # shift left because index 12 to 5
#         sw      t3,0(t0)             # write to LEDs
#         j       loop                 # repeat loop forever
#
# THE EXAMINER'S ANSWER HAS MISTAKES. This is not forced to match:
#   1. slli t3,t3,5 shifts t3 ITSELF. t3 is also the remembered value.
#      When A is released t3 is not set to 13 again, so on the next round
#      the old 13<<5 is shifted 5 more steps and the LEDs no longer show
#      13 (the port gets 0x3400 instead of 0x1a0). The question says 13
#      must stay. Fix: shift into another register:  slli t5,t3,5 / sw t5,0(t0).
#   2. Typing slips:  ori t0, 0x010  needs three operands
#      (ori t0,t0,0x010), the same for  ori t4, 0x440,  and
#      beq t2,0,notA  must be  beq t2,zero,notA.
#
# WRITE THIS ON THE EXAM (the examiner's answer with those fixes):
#         lui     t0, 0x17880
#         ori     t0, t0, 0x010        # t0 = 0x17880010 (LEDs)
#         addi    t3, zero, 0          # t3 = the value to show. 0 at the start
#         lui     t4, 0xffffe
#         ori     t4, t4, 0x440        # t4 = 0xffffe440 (buttons)
#     loop:
#         lw      t1, 0(t4)            # read the buttons
#         andi    t2, t1, 0x80         # button A (bit 7)
#         beq     t2, zero, notA
#         addi    t3, zero, 13         # A pushed: remember 13
#     notA:
#         andi    t2, t1, 0x10         # button D (bit 4)
#         beq     t2, zero, notD
#         addi    t3, zero, 0          # D pushed: remember 0 (D is tested last, so D wins over A)
#     notD:
#         slli    t5, t3, 5            # LEDs start at bit 5
#         sw      t5, 0(t0)            # write to the LEDs
#         j       loop
#
# THE LOOP MUST END HERE: the exam's loop is endless. Here it runs once
# for each value in the list seq12 (the buttons a "user" pushes):
# nothing, A, nothing, D, A, A+D. After each round the LED port is
# printed. 13 << 5 = 416 = 0x1a0.
item12b:
        mv   s11, ra
        la   a0, s12b
        jal  print_str
        jal  print_newline
        la   t0, led12             # <-- fake port.  exam: lui t0,0x17880 / ori t0,t0,0x010
        addi t3, zero, 0           # the value to show. 0 at the start
        la   t4, btn12             # <-- fake port.  exam: lui t4,0xffffe / ori t4,t4,0x440
        la   s0, seq12             # (not in the exam) s0 points at the list of button values
        addi s1, zero, 6           # (not in the exam) 6 rounds
loop12:
        lw   t5, 0(s0)             # (not in the exam) the "user" pushes buttons:
        sw   t5, 0(t4)             # (not in the exam)   put the next value in the button port
        lw   t1, 0(t4)             # read the buttons
        andi t2, t1, 0x80          # button A (bit 7)
        beq  t2, zero, notA
        addi t3, zero, 13          # A pushed: remember 13
notA:
        andi t2, t1, 0x10          # button D (bit 4)
        beq  t2, zero, notD
        addi t3, zero, 0           # D pushed: remember 0
notD:
        slli t5, t3, 5             # LEDs start at bit 5
        sw   t5, 0(t0)             # write to the LEDs
        lw   a0, 0(t0)             # (not in the exam) print the LED port
        jal  print_hex
        addi s0, s0, 4             # (not in the exam) next button value
        addi s1, s1, -1
        bne  s1, zero, loop12      # <-- exam: j loop
        jr   s11

# ===================== Item 13 (b) =====================
# 2020-06-05-riscv Q2.pdf   (same as Q2.6code.png and Q2.1translate.png)
# Translate to assembly, without pseudo instructions:
#     volatile int* leds = (volatile int*) 0x1dbb8050;
#     volatile int* switches = (volatile int*) 0x22430030;
#     *leds = (*leds & 0x79f) | ((((*switches) >> 4) & 0x3) << 3);
# (The C version runs in module2_io.c, item13b, with the same values.)
#
# The examiner's answer, exactly as printed:
#     lui t2,0x1dbb8
#     ori t2,0x050
#     lui t1,0x22430
#     ori t1,0x030
#     lw t0,0(t1)
#     addi t4, zero, 4
#     srl t0,t0,t4
#     andi t0,t0,0x3
#     addi t4, zero, 3
#     sll t0,t0,t4
#     lw t3,0(t2)
#     andi t3,t3,0x79f
#     or t3,t3,t0
#     sw t3,0(t2)
# Typing slip in the key: ori needs three operands. Write
#     ori t2,t2,0x050      and      ori t1,t1,0x030
# The key puts the shift amount in t4 and uses srl / sll. The shorter
#     srli t0,t0,4         and      slli t0,t0,3
# do the same thing and are real instructions too.
# 0x79f fits in andi's 12 bits (it is below 0x800), so no extra register
# is needed for the mask here. Compare with item 7(b).
item13b:
        mv   s11, ra
        la   t2, led13             # <-- fake port.  exam: lui t2,0x1dbb8 / ori t2,t2,0x050
        la   t1, sw13              # <-- fake port.  exam: lui t1,0x22430 / ori t1,t1,0x030
        lw   t0, 0(t1)             # t0 = *switches            0x20
        addi t4, zero, 4
        srl  t0, t0, t4            # >> 4                      2
        andi t0, t0, 0x3           # & 0x3                     2
        addi t4, zero, 3
        sll  t0, t0, t4            # << 3                      0x10
        lw   t3, 0(t2)             # t3 = *leds                0xfe7
        andi t3, t3, 0x79f         # & 0x79f                   0x787
        or   t3, t3, t0            # |                         0x797
        sw   t3, 0(t2)             # *leds = 0x797
        la   a0, s13b
        jal  print_str
        la   t2, led13
        lw   a0, 0(t2)             # read it back to check
        jal  print_hex
        jr   s11

# ===================== extra_Q2_15 =====================
# Q2.15create.png   (older exam, not one of the 13 sittings)   ORIGINAL WAS MIPS
# 8 LEDs are the 8 lowest bits of the port at 0xbf226000. LED #1 is
# bit 0 ... LED #8 is bit 7. Write a function led4: the LOWEST bit of
# a0 says if LED #4 is turned on (1) or off (0). The other bits of a0
# can be anything, so they must be masked away. Other LEDs must not
# change. No pseudo instructions.
#
# Examiner's MIPS answer:
#     led4: lui  $t0,0xbf22
#           ori  $t0,$t0,0x6000
#           andi $a0,$a0,1          # mask so that we only use bit 0
#           sll  $a0,$a0,3          # shift the value to LED 4 position
#           lw   $t1,0($t0)         # load the previous setting
#           andi $t1,$t1,0xfff7     # clear bit 3
#           or   $a0,$t1,$a0        # insert the bit for the led
#           sw   $a0,0($t0)         # write to the memory mapped IO
#           jr   $ra
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     led4: lui  t0, 0xbf226        # t0 = 0xbf226000 (the last 3 hex digits are 000)
#           andi a0, a0, 1          # keep only bit 0 of the argument
#           slli a0, a0, 3          # LED #4 is bit 3 (LED #1 is bit 0)
#           lw   t1, 0(t0)          # read the port
#           andi t1, t1, -9         # clear bit 3   (-9 = 0xfffffff7)
#           or   a0, t1, a0         # put the new bit 3 in
#           sw   a0, 0(t0)          # write the port
#           jr   ra
extra_Q2_15:
        mv   s11, ra
        la   a0, sx15
        jal  print_str
        jal  print_newline
        addi a0, zero, -1          # a0 = 0xffffffff: lowest bit 1, and "rubbish" in the other bits
        jal  led4                  # LED #4 on    0xa5 -> 0xad
        la   t4, portx15
        lw   a0, 0(t4)
        jal  print_hex
        addi a0, zero, -2          # a0 = 0xfffffffe: lowest bit 0
        jal  led4                  # LED #4 off   0xad -> 0xa5
        la   t4, portx15
        lw   a0, 0(t4)
        jal  print_hex
        jr   s11

led4:
        la   t0, portx15           # <-- fake port.  exam: lui t0, 0xbf226
        andi a0, a0, 1             # keep only bit 0 of the argument
        slli a0, a0, 3             # move it to bit 3 (LED #4)
        lw   t1, 0(t0)             # read the port
        andi t1, t1, -9            # clear bit 3, keep all other bits
        or   a0, t1, a0            # put the new bit 3 in
        sw   a0, 0(t0)             # write the port
        jr   ra

# ===================== extra_Q2_16 =====================
# Q2.16create.png   (older exam, there called Module 5)   ORIGINAL WAS MIPS (or NIOS II)
# Show the digit 6 on a 7-segment display. The display is a 32-bit port
# at 0x800020f0. The figure gives the bit number of each segment:
#
#        0
#      -----
#   5 |     | 1          A 6 is every segment except 0 and 1:
#     |  6  |            bits 6 5 4 3 2 on  =  111 1100  =  0x7c.
#      -----             (With the top segment too: 111 1101 = 0x7d.
#   4 |     | 2           The key accepts both.)
#     |     |
#      -----
#        3
#
# Here the whole port is simply written. Nothing has to be kept, so no
# lw is needed.
#
# Examiner's MIPS answer:
#     lui  $t1,0x8000
#     addi $t1,$t1,0x20f0
#     addi $t0,$0,0x7c
#     sw   $t0,0($t1)
#
# WRITE THIS ON THE EXAM (RISC-V, this file's own translation):
#     lui  t1, 0x80002             # the 5 first hex digits of 0x800020f0
#     addi t1, t1, 0xf0            # the 3 last hex digits.   t1 = 0x800020f0
#     addi t0, zero, 0x7c          # the pattern for a 6
#     sw   t0, 0(t1)               # write it to the display
extra_Q2_16:
        mv   s11, ra
        la   t1, portx16           # <-- fake port.  exam: lui t1,0x80002 / addi t1,t1,0xf0
        addi t0, zero, 0x7c        # the pattern for a 6
        sw   t0, 0(t1)             # write it to the display
        la   a0, sx16
        jal  print_str
        la   t1, portx16
        lw   a0, 0(t1)             # read it back to check
        jal  print_hex
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
