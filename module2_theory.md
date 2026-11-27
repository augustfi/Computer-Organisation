# Module 2, Q2: I/O Systems - theory questions

These are the questions that cannot be run as code: every true/false statement
with the examiner's answer, and the questions that are answered in words.
They are grouped by exam sitting, with the same item numbers as the code files.

On the exam: if you say a statement is **false** you must write a short reason
(at most 2 sentences). If it is **true**, no reason is needed.

- "Key" is the examiner's answer, copied from the official solution.
- "Why" under a **true** statement is this file's own short explanation. The
  examiner gives no motivation for true statements.

Where the other Module 2 questions are:

| File | What is in it |
|---|---|
| `module2_timer.c` | timer calculations: items 1(b), 3(c), 6(b), 8(b), 9(a) i, 11(b), 12(a), 13(a) |
| `module2_io.c` | C and memory-mapped I/O: items 2(b), 4(a), 7(a), 10(a) i/iii/iv, 13(b), extra_Q2_14, extra_Q2_17, extra_Q2_19 |
| `module2_io.asm` | assembly: items 1(a), 2(b), 3(a), 4(b), 5(a), 6(a), 7(b), 8(a), 9(b), 10(a) ii, 11(a), 12(b), 13(b), extra_Q2_15, extra_Q2_16 |

---

## Item 2 (a) - 2017-01-12 Q2.pdf

**i.** Polling means that a program checks over and over again if a value has
changed, whereas interrupts make it possible to be notified when it has changed.

- Key: **True.**
- Why: polling is a loop that reads the port again and again (see `check_button`
  in `module2_io.asm`, item 5(a)). With an interrupt the hardware stops the
  program and runs a special function when something happens.

**ii.** One important property of an asynchronous bus is that communicating
entities are synchronized using a common clock signal.

- Key: **False.** A synchronous bus is using a common clock signal.

**iii.** Direct memory access (DMA) is a clever way for the programmer to get
direct file access to the hard drive.

- Key: **False.** DMA is used to offload the processor, by automatically
  transferring data between different parts of the main memory. It has nothing
  to do with file access to the hard drive.

**iv.** The `volatile` keyword in the C programming language is important to use
together with pointers when accessing data from memory-mapped I/O ports.

- Key: **True.**
- Why: a port can change by itself (somebody pushes a button). `volatile` makes
  the compiler really read or write the port every time, instead of optimising
  the access away.

## Item 3 (b) - 2017-03-16 Q2.pdf

**i.** If an interrupt service routine (ISR) for a timer interrupt is
implemented correctly, the ISR can be executed periodically without affecting
the behavior (except timing behavior) of the main program.

- Key: **True.**
- Why: a correct ISR saves every register it uses and puts them back before it
  returns. The main program continues as if nothing happened, only a little later.

**ii.** One important use case for the `volatile` keyword in C programs is to
instruct the C compiler to avoid certain optimizations, to make sure that memory
mapped I/O operations behave correctly.

- Key: **True.**

## Item 4 (c) - 2017-06-08 Q2.pdf

**i.** Direct memory access (DMA) makes it possible to perform memory transfer
while the processor can perform other computational tasks.

- Key: **True.**
- Why: a separate DMA controller does the copying, so the processor is free.

**ii.** A synchronous bus makes use of a common clock signal that is available
at both the sender and receiver side.

- Key: **True.**

## Item 5 (b) - 2018-01-15 Q2.pdf

**i.** The keyword `volatile` in the C programming language is a built in
synchronization primitive that is especially important for concurrent programming.

- Key: **False.** The `volatile` keyword is not used as a synchronization
  primitive. It makes sure that certain compiler optimizations are not performed
  in unsafe places.

**ii.** An asynchronous bus makes use of a common clock that is available at
both the sender and receiver side.

- Key: **False.** This is the principle of a synchronous bus, and not an
  asynchronous bus.

**iii.** The period register in a timer is stating the value when the timer
should wrap around, and start to count from zero again.

- Key: **True.**
- Why: this is the definition used in every calculation in `module2_timer.c`.

## Item 6 (b), second half - 2018-03-15 Q2.pdf

How could the prescaling factor be changed, to make the LED light twinkle twice
as fast? (The timer has prescaling 1:8.)

- Key: **Prescaling factor 1:4.** "If we want to double the frequency of
  twinkling the LED light, the prescaling factor must be changed to 1:4."
- Why: half the prescale means the timer counts twice as fast, so it wraps twice
  as often. The numbers are worked out in `module2_timer.c`, item 6(b).

## Item 7 (c) - 2018-06-05 Q2.pdf

**i.** Direct memory access (DMA) is a technique in the C programming language
to get direct access to the main memory using pointers.

- Key: **False.** DMA is an efficient approach to copy data between different
  parts of the memory. It has nothing to do with pointers in C.

**ii.** An asynchronous bus does not need a clock signal between the sender and
the receiver.

- Key: **True.**
- Why: an asynchronous bus has no shared clock. (The usual way to do without
  one is extra "handshake" signals; the key itself gives no reason.)

## Item 9 (a) - 2019-03-15-riscv Q2.pdf (same as Q2.2clock.png)

**i.** Suppose you are configuring a 16-bit timer running at 100 MHz. You would
like your program to write out a message to the standard output every 10th
second. It is clear to you that you cannot configure the period register so that
the timer triggers every 10th second. Even if this limitation exists, it is
still possible to use the 16-bit timer to solve the described task using
software counters.

- Key: **True.**
- Why: let the timer wrap often and count the wraps in an ordinary variable.
  The numbers are worked out in `module2_timer.c`, item 9(a) i.

**ii.** The `volatile` keyword in the C programming language means that the
specified memory segment is volatile and that the data will be lost if the
computer is powered off.

- Key: **False.** The `volatile` keyword in C can be used to declare a volatile
  pointer, which means that the content to which the pointer points to can be
  changed at any time. It has nothing to do with volatile memories.

**iii.** A key aspect of a synchronous bus compared to an asynchronous bus is
that a synchronous bus is using a clock signal.

- Key: **True.**

**iv.** When interrupts are used in an embedded system, it means that a program
can be interrupted, but it *cannot* jump back to the original program after that
the interrupt is over.

- Key: **False.** An interrupt makes it possible to interrupt and execute some
  other piece of code, and then safely go back to the original code and continue
  execution.

## Item 10 (a) i, iii, iv - 2019-06-05-riscv Q2.pdf (same as Q2.3code.png, Q2.4translate.png)

The function `getvals()` loops `while(*pushbutton & 1)` and inside the loop does
`v = (*switches >> 4) & 0xf;`. The code runs in `module2_io.c`, item 10(a).

**i.** The C code is missing a keyword to work correctly. Which keyword?

- Key: The pointers should be declared using the **`volatile`** keyword.

**iii.** What will trigger the function to exit?

- Key: The function will exit when **push button number 1 is released** (not pushed).

**iv.** What value will be returned?

- Key: When push button 1 is released, **the 4-bit value formed by the status of
  switches 4, 3, 2, and 1** is returned.

## extra_Q2_18 - Q2.18.png (one statement from a longer true/false list, sitting unknown)

A memory-mapped general-purpose I/O (GPIO) pin can be configured to be either an
output or input port, by writing a configuration parameter to a specific memory
address that is dedicated for configuring this GPIO pin.

- Key: **True.**
- Why: this is why several exam questions say "you can assume that the GPIO is
  already configured as an output port".

---

## The facts behind all the statements

| Topic | What to remember |
|---|---|
| Polling | The program checks again and again. |
| Interrupt | The program is notified. It is interrupted, other code runs, then it safely continues where it was. |
| ISR | If written correctly, it changes nothing for the main program except timing. |
| `volatile` | Tells the compiler not to optimise accesses: the value can change at any time. Needed for memory-mapped I/O pointers. **Not** about losing data at power off. **Not** a synchronization primitive. |
| Synchronous bus | Sender and receiver share a **common clock** signal. |
| Asynchronous bus | **No** common clock. |
| DMA | Copies data between parts of memory **without the processor doing it**, so the processor can do other work. **Not** file access to the hard drive. **Not** C pointers. |
| Period register | The value where the timer wraps around and starts from zero again. |
| Software counter | A variable that counts timer wraps, to measure times longer than the timer can hold. |
| GPIO pin | Made an input or an output by writing to a configuration address. |
