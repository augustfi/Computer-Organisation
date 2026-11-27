# Module 6: Parallel Processors and Programs — the questions that cannot be run

This file holds the true/false statements and the explain/classify questions of Q6.
The calculations are in `module6_amdahl.c` (Amdahl's law) and `module6_misc.c` (AVX width, CPI).

How to read it:

- **Part 1** lists every statement **by exam sitting**, with the examiner's answer and motivation.
- **Part 2** lists the same statements again **by topic**, because the same ideas come back in many sittings.
- Item numbers are the sittings in date order (1 = 2016-06-09 ... 13 = 2020-06-05), the same numbers as in the C files.
- "Key" is the examiner's answer. The motivation is the examiner's, put in plain words. A true statement needs no motivation on the exam, so most true statements have none.
- On the exam: a false answer only gives the point if you also say **why** it is false (at most two sentences).

---

## Words you need first

| Word | Plain meaning |
|---|---|
| **SISD** | Single Instruction, Single Data. One stream of instructions works on one stream of data. Every ordinary single-core processor. |
| **SIMD** | Single Instruction, Multiple Data. One instruction does the same operation on several data items at once. This is **data-level parallelism**. |
| **MIMD** | Multiple Instruction, Multiple Data. Several instruction streams, each with its own data. A multicore processor. This is **task-level parallelism**. |
| **MISD** | Multiple Instruction, Single Data. Hardly ever used. |
| **AVX** | Advanced Vector Extension. Intel's SIMD instructions for x86. Also called a multimedia extension or subword parallelism. |
| **ILP** | Instruction-Level Parallelism. The processor runs several instructions of ONE program at the same time (pipelining, multiple issue). |
| **Multiple issue** | Starting more than one instruction in each clock cycle. A form of ILP. |
| **VLIW** | Very Long Instruction Word. Multiple issue where the **compiler** decides, before the program runs, which instructions go together (**static** scheduling). |
| **Superscalar** | Multiple issue where the **processor hardware** decides while the program runs (**dynamic** scheduling). |
| **Out-of-order execution** | The processor changes the order of instructions while running, as long as the result stays the same. |
| **Register renaming** | A trick used in out-of-order processors to remove hazards that come from reusing the same register name. |
| **Hardware multithreading** | One core switches between several hardware threads, for example while one of them waits for memory. Its purpose is **hiding latencies** (hiding waiting time) and avoiding stalls. |
| **SMT** | Simultaneous multithreading. Hardware multithreading on a superscalar processor. Intel's name for it is **hyper-threading**. |
| **Software thread** | A thread made by a program and scheduled by the **operating system**. |
| **Concurrency** | Several things are in progress at once (they may take turns on one core). |
| **Parallelism** | Several things really run at the same instant (needs more than one core). |
| **Semaphore / mutex** | Software tools for synchronizing threads. A mutex (mutual exclusion) lets one thread at a time into a critical section. A mutex is a binary semaphore. |
| **SMP** | Shared memory multiprocessor. Several cores share the same memory. The usual kind of multicore. |
| **Cache coherence problem** | Cores have their own caches, so two cores can see different values for the same memory address. |
| **Snooping protocol** | One way to solve the cache coherence problem. |
| **False sharing** | Two threads use different data that happen to lie in the same cache block (cache line), so they keep invalidating each other's cache without sharing any data. |
| **Strong scaling** | Measure speedup while the problem size stays fixed. |
| **Weak scaling** | Measure speedup while the problem size grows together with the number of cores. |
| **Moore's law** | The number of transistors on a chip doubles about every 18 to 24 months. |
| **MapReduce** | A programming model for parallel computations on clusters / warehouse-scale computers. |

---

# Part 1 — By exam sitting

## Item 1 (a) — 2016-06-09 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | The benefit with VLIW processors is that all scheduling of parallel instructions is done automatically by the processor in hardware. | **False** | VLIW uses static scheduling done by the compiler at compile time. The processor does not do the scheduling. |
| ii | In modern operating systems, it is common that software threads are scheduled on different cores. | **True** | |
| iii | A semaphore is a software construct that enables explicit instruction level parallelism (ILP). | **False** | A semaphore is used for synchronization in concurrent programming. |
| iv | Hardware multithreading is a technique that enables fine-grained fetching of multiple instructions in parallel. | **False** | Hardware multithreading means that the processor switches between different hardware threads. It does not fetch multiple instructions. It can improve performance by hiding latencies. |
| v | Shared memory multiprocessors (SMP) are today one of the most common types of multicore processors. | **True** | |

## Item 2 (b) — 2017-01-12 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | SIMD (single instruction multiple data) is a form of data-level parallelism. | **True** | |
| ii | Instruction-level parallelism (ILP) is not used in modern processors because it does not give any performance gains anymore. | **False** | ILP is very common in modern processors. It gives good performance gain compared to not using ILP. |
| iii | The main benefit with superscalar processors is that they are extremely energy efficient because all scheduling of parallel instructions are done by the compiler. | **False** | What is described is VLIW. A superscalar processor does the scheduling in hardware. |
| iv | Graphical processing units (GPUs) use hardware multithreading as a technique for hiding latencies. | **True** | |
| v | A mutex is a binary semaphore that is useful in concurrent programming. | **True** | See the note below. |

Note from the key on (v): "False" is also accepted with a valid motivation, for example that a mutex is a general
concept and does not have to be built from a binary semaphore. (A binary semaphore is still a common way to build a
mutex.) The original exam had the typo "current" instead of "concurrent"; an answer of false because of the typo was
also accepted.

## Item 3 (a) — 2017-03-16 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Weak scaling means that the experiment was performed on few computers with weak processing power. | **False** | Weak scaling means that the data size is also increased when the number of processors/cores is increased. |
| ii | SIMD can work for applications that can make use of data-level parallelism. | **True** | |
| iii | A processor that supports VLIW is designed to utilize instruction level parallelism (ILP). | **True** | |
| iv | Register renaming is a way to solve hazards due to out-of-order execution. | **True** | |
| v | Hyper-threading is Intel's approach for scheduling software threads efficiently within the operating system. | **False** | Hyper-threading is Intel's name and implementation of simultaneous multithreading (SMT). |

## Item 4 (b) — 2017-06-08 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | For strong scaling, the speedup of a program is estimated by keeping the program size fixed. | **True** | |
| ii | When solving data hazards, register renaming is another name for forwarding. | **False** | Register renaming is used for solving hazards due to out-of-order execution. |
| iii | Today, all high-performance commercial processors have between 4 and 6 pipeline stages. | **False** | Today it is very common with many more pipeline stages, e.g. 14 pipeline stages for an Intel Core i5. |
| iv | AVX (Advanced Vector Extension) is a technique especially designed to handle task-level parallelism. | **False** | AVX is used for data-level parallelism. |
| v | One of the main problems of hardware multithreading is cache coherence. | **False** | Fine-grained multithreading is not related to the cache coherence problem. The cache coherence problem appears in a multicore processor with separate caches. |

## Item 5 (b) — 2018-01-15 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | AVX (Advanced Vector Extension) is an advanced form of hardware threading. | **False** | AVX is an extension for SIMD. It has nothing to do with hardware threading. |
| ii | Multiple Instruction Single Data (MISD) is a useful technique for instruction-level parallelism. | **False** | MISD is not used very much, because it is unclear what multiple instructions on a single data item would mean for parallelism. It has nothing to do with ILP. |
| iii | Semaphores are useful when developing concurrent programs. | **True** | |
| iv | Unrolling a loop is a technique that can improve the performance by better utilizing instruction-level parallelism. | **True** | |
| v | Hyper-threading is another name for simultaneous multithreading. | **True** | |

## Item 6 (a) — 2018-03-15 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | A concurrent program can potentially be scheduled to be executed in parallel by an operating system, when the operating system is executing on a multicore processor. | **True** | |
| ii | Advanced Vector Extension (AVX) is an example of Single Instruction Multiple Data (SIMD). | **True** | |
| iii | Superlinear speedups are very common in practice, especially on superscalar processors. | **False** | Superlinear speedup is very uncommon, and such results often mean that something is wrong in the measurements. The "super" in superscalar has nothing to do with the "super" in superlinear. |
| iv | Some of the main motivations behind hardware multithreading are to hide latencies and to avoid stalls. | **True** | |
| v | The VLIW technique performs instruction scheduling dynamically in hardware. | **False** | Typically the compiler does the scheduling for VLIW processors statically, before the program is executed. |

## Item 7 (b) — 2018-06-05 Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Single Instruction Single Data (SISD) is a very unusual approach and there have been extremely few examples of SISD processors in the past. | **False** | SISD has been and is still very common. It represents all single core processors with one instruction stream that works on one stream of data. |
| ii | Moore's law means that the number of transistors doubles approximately every second year. | **True** | |
| iii | Multiple issue is one form of instruction-level parallelism (ILP). | **True** | |

Item 7 (c) (AVX `vaddpd` with 256-bit registers: 4 doubles in parallel) is a calculation, see `module6_misc.c`.

## Item 8 (a) — 2019-01-12-riscv Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Advanced Vector Extension (AVX) is designed for programs with data-level parallelism. | **True** | |
| ii | Hardware multithreading is a form of concurrency mechanism inside a process of an operating system. | **False** | Hardware threads are scheduled by the hardware and not by the operating system. |
| iii | Hyper-threading is another name for Very Long Instruction Words (VLIW). | **False** | Hyper-threading is another name for simultaneous multithreading, whereas VLIW is a form of multiple issue. |
| iv | Instruction-level parallelism is a parallelization approach that is available in almost all modern processors for laptops and workstations. | **True** | |
| v | Single Instruction Multiple Data (SIMD) is often used in applications with task-level parallelism. | **False** | SIMD is used when there is data-level parallelism. |

## Item 9 (b) — 2019-03-15-riscv Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | A Linux operating system running on a processor with just one core is a good example of running programs with concurrency, but without parallelism. | **True** | |
| ii | A multiplexer can be used to avoid that several different threads in a multithreaded program access the same resource. | **False** | A mutex is useful for this, not a multiplexer (which is used in digital design). |
| iii | MapReduce is a programming model that is commonly used for parallel computations on cluster systems. | **True** | |
| iv | False sharing is a problematic situation on a multicore system, where different cores invalidate the same part of the cache, without actually accessing the same data. | **True** | |
| v | One important component of simultaneous multithreading (SMT) is fine-grained multithreading. | **True** | See the note below. |

Note from the key on (v): "False" is also accepted as long as the argument is correct. The argument can be either
that it needs to be hardware multithreading, or that it does not have to be fine-grained multithreading.

## Item 10 (b) — 2019-06-05-riscv Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | SIMD (single instruction, multiple data) corresponds to the notion of data-level parallelism. | **True** | |
| ii | Software threads are scheduled by the operating system. | **True** | |
| iii | A semaphore is a clever way to avoid the cache coherence problem. | **False** | Semaphores can be used to handle e.g. shared resources in concurrent programming, whereas the cache coherence problem is a problem that occurs in multicore systems with caches. |
| iv | Advanced Vector Extension (AVX) is a SIMD standard used in x86 processors. | **True** | |
| v | VLIW processors require that instruction scheduling is done statically, at compile time. | **True** | |

## Item 11 (b) — 2020-01-13-riscv Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Hardware multithreading is typically used for hiding latencies. | **True** | |
| ii | SIMD (single instruction, multiple data) corresponds to the notion of thread-level parallelism. | **False** | SIMD corresponds to data-level parallelism. |
| iii | Advanced Vector Extension (AVX) is a standard for handling instruction-level parallelism (ILP). | **False** | AVX is a standard for handling SIMD. It is not used for ILP. |
| iv | One way of handling the cache coherency problem is to use the snooping protocol. | **True** | |
| v | MapReduce is an efficient way for handling Very Long Instruction Words (VLIW). | **False** | MapReduce is a programming model, typically used in distributed computing. VLIW is a way to achieve instruction level parallelism (ILP). |

## Item 12 (a) — 2020-03-11-riscv Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Data-level parallelism means that one instruction operates on one data item. | **False** | In data-level parallelism, each instruction operates on several data items. |
| ii | Advanced Vector Extension (AVX) is a well-known example of hardware multithreading. | **False** | AVX is a standard for SIMD instructions and has nothing to do with hardware multithreading. |
| iii | Hardware multithreading results in concurrency and not in parallelism. | **True** | |
| iv | Very Long Instruction Word (VLIW) is a form of instruction-level parallelism (ILP). | **True** | |
| v | False sharing means that two software threads share the same page in virtual memory. | **False** | False sharing means that two different threads use the same cache line, which results in invalidation of the cache. It has nothing to do with virtual memory. |

## Item 13 (b) — 2020-06-05-riscv Q6.pdf

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Register renaming is a way to solve control hazards in single-cycle processors. | **False** | Register renaming is used in out-of-order processors. |
| ii | Hyper-threading is Intel's name for simultaneous multithreading (SMT). | **True** | |
| iii | AVX (Advanced Vector Extensions) can be applicable in an application that has data-level parallelism. | **True** | |
| iv | Software threads are typically used when implementing SIMD (Single instruction, multiple data) instructions. | **False** | Software threads are used in an operating system to enable concurrency and are not related to SIMD instructions. |
| v | Semaphores can be used to synchronize software threads. | **True** | |

---

## Questions that exist only as a picture (not from one of the 13 sittings)

The answers below are the examiner's, read from the picture itself.

### extra_Q6_TF_Q7 (a) — Q6-TF-Q7.png

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | AVX (Advanced Vector Extension) is the most common technique today for implementing instruction level parallelism (ILP). | **False** | AVX is a standard for implementing SIMD or subword parallelism. |
| ii | Moore's law says that your computer will get twice as hot every second year. We have already reached this limit. | **False** | Moore's law basically says that the number of transistors on an integrated circuit doubles every 18-24 months. |
| iii | In concurrent programs, semaphores can be used to make sure that shared resources are accessed in a correct way. | **True** | |
| iv | The main benefit with multicore processors are that they use data-level parallelism. That is, all cores in the processor execute the same instruction sequence, but with different data items. | **False** | A multicore processor uses task-level parallelism. Each core has a different instruction stream (not the same instruction stream as in data-level parallelism). |

### extra_Q6_TF_Q8 (b) — Q6-TF-Q8.png

| # | Statement | Key | Motivation |
|---|---|---|---|
| i | Moore's law stopped working around year 2006 because the processors got too hot. | **False** | Moore's law still holds (but we do not know for how long). It is not about the power problem; it says that the number of transistors doubles every 18-24 months. |
| ii | SIMD instructions are suitable to use if the program includes data-level parallelism. | **True** | |
| iii | Multicore is the only real important parallel construct in modern computers that can give great speedup. | **False** | SIMD instructions can also greatly affect performance. |
| iv | Intel's Hyper-threading technology is making use of hardware multithreading. | **True** | |
| v | A major challenge for shared memory multicore processors is the cache coherence problems, which means that different cores can see different values for the same memory address. | **True** | |

### extra_Q6_TF_Q10 (a) — Q6-TF-Q10.png — VLIW versus superscalar

**Question:** What is the main difference between VLIW (very long instruction word) processors and superscalar
processors? At most two sentences. (2 points)

**Key:** Superscalar processors schedule instructions in parallel **dynamically**, whereas a **compiler** needs to
schedule the instructions **statically at compile time** for a VLIW processor.

How the points are given: 1 point if it is clear that superscalar processors schedule instructions dynamically in
hardware. 1 point if it is clear that VLIW needs the instructions scheduled at compile time (by the compiler).

### extra_Q6_TF_Q10 (b) — Q6-TF-Q10.png — assign SIMD / MIMD / Hiding latencies / ILP

**Question:** For each item, assign the acronym or concept that fits best: SIMD, MIMD, Hiding latencies, ILP. (3 points)

| # | Item | Key |
|---|---|---|
| i | Multicore | **MIMD** |
| ii | VLIW (Very Long Instruction Word) | **ILP** |
| iii | Multimedia extension | **SIMD** |
| iv | Data-level parallelism | **SIMD** |
| v | Hardware multithreading | **Hiding latencies** |
| vi | Multiple issue (instructions) | **ILP** |

Each item gives 0.5 points, rounded upwards.

### extra_Q6_TF_Q11 (a) — Q6-TF-Q11.png — what kind of parallelism

**Question:** A processor has an add instruction that executes in one clock cycle but can add together 4 different
16-bit words in parallel. What kind of parallelism is this? There is more than one correct answer; one is enough.
(2 points)

**Key:** Subword parallelism, or single-instruction multiple-data (SIMD), or data-level parallelism.

A small demo of the idea is in `module6_misc.c` (`extra_Q6_TF_Q11_a`).
Sub-question (b) is not in the picture. Sub-question (c) (CPI = 18/27 = 2/3) is a calculation, see `module6_misc.c`.

### extra_Q6_TF_Q12 (b) — Q6-TF-Q12.png — true / false / don't know

This is from an older exam format where a wrong answer gives minus points and no motivation is asked for.

| # | Statement | Key | Motivation |
|---|---|---|---|
| 1 | A good example of MIMD is a modern multicore processor. | **True** | |
| 2 | MapReduce can be based on message passing techniques and is today used extensively in Warehouse-scale computers. | **True** | |
| 3 | If a semaphore is used for mutual exclusion (mutex), it means the programmer can define critical sections by first locking a mutex before entering the critical section, and then unlocking the mutex when exiting the critical section. Hence, a mutex can be used to have controlled access to shared resources in a concurrent environment. | **True** | |
| 4 | Assume we have a shared memory processor (SMP) consisting of 4 cores with separate L1 caches and an L2 cache that is shared among the cores. Assume further that two of the cores frequently read or write to the same address in memory. As a consequence, we get easily inconsistencies in the L1 caches. This phenomenon is called *false sharing*. | **False** | The problem described is related to cache coherence, but not directly to false sharing. If it was about false sharing, the cores would write to the same cache block, but not to the exact same address. |

### extra_Q6_TF_Q13 (b) — Q6-TF-Q13.png — kind of dependency

**Question:** The following MIPS code is executed using an out-of-order processor. What kind of dependency is this
and what is the dependency? (2 points)

```
lw  $t1, 16($t0)
sub $t0, $t3, 10
```

**Key:** This is a **Write after Read (WAR)** dependency, where register `$t0` is used (read) in the `lw` instruction
and then written to in the `sub` instruction.

Plain words: `lw` must read the OLD value of `$t0` before `sub` puts a NEW value into `$t0`. An out-of-order
processor may want to run `sub` first, and then `lw` would read the wrong address. Register renaming solves this:
the processor lets `sub` write to another physical register.

The exam is RISC-V only now. The same two lines in RISC-V would be (the dependency is the same, on `t0`):

```
lw   t1, 16(t0)
addi t0, t3, -10
```

The kinds of dependency, for comparison (the first line is the key's; the other two are general knowledge, not from this key):

| Name | Meaning |
|---|---|
| **WAR** (write after read) | A later instruction writes a register that an earlier instruction reads. This question. |
| RAW (read after write) | A later instruction reads a register that an earlier instruction writes. The ordinary data hazard. |
| WAW (write after write) | Two instructions write the same register. |

### extra_Q6_TF_Q13 (c) — Q6-TF-Q13.png — true / false / don't know

| # | Statement | Key | Motivation |
|---|---|---|---|
| 1 | An out-of-order processor can rearrange the order that instructions are executed dynamically, as long as the result of the program execution does not change. | **True** | |
| 2 | Data-level parallelism and MIMD have much in common, especially in that one stream of instructions is used to operate on multiple instances of data. | **False** | MIMD stands for multiple instruction, multiple data and has multiple instruction streams. |
| 3 | One of the main uses of hardware multithreading is to increase throughput by switching threads when the processor needs to wait on, for instance, memory requests. | **True** | |
| 4 | If two cores in a multicore processor are reading from the same memory address in the main memory, and one of the cores always gets the data much faster than the other core, we have an example of a cache coherence problem. | **False** | The cache coherence problem means that different cores' local caches contain different values for the same memory address. This may be the result of a write by one of the cores. Reading from the same memory address is not a cache coherence problem in itself. |

---

# Part 2 — By topic

The same statements again, sorted by idea. **T** = true, **F** = false. The reference at the end of each line says
where it comes from in Part 1 (for example "3 a ii" = Item 3 (a), statement ii; "Q7 i" = extra_Q6_TF_Q7, statement i).

## SIMD and AVX

The one thing to remember: **SIMD = AVX = multimedia extension = subword parallelism = data-level parallelism.**
One instruction, several data items. It is NOT ILP, NOT hardware multithreading, NOT task/thread-level parallelism,
and it has nothing to do with software threads.

- **T** — SIMD is a form of data-level parallelism. (2 b i)
- **T** — SIMD can work for applications that can make use of data-level parallelism. (3 a ii)
- **T** — SIMD corresponds to the notion of data-level parallelism. (10 b i)
- **T** — SIMD instructions are suitable if the program includes data-level parallelism. (Q8 ii)
- **T** — AVX is an example of SIMD. (6 a ii)
- **T** — AVX is designed for programs with data-level parallelism. (8 a i)
- **T** — AVX is a SIMD standard used in x86 processors. (10 b iv)
- **T** — AVX can be applicable in an application that has data-level parallelism. (13 b iii)
- **F** — AVX is especially designed to handle task-level parallelism. *AVX is used for data-level parallelism.* (4 b iv)
- **F** — AVX is an advanced form of hardware threading. *AVX is an extension for SIMD.* (5 b i)
- **F** — AVX is a well-known example of hardware multithreading. *AVX is a standard for SIMD instructions.* (12 a ii)
- **F** — AVX is a standard for handling ILP. *AVX is a standard for SIMD, not ILP.* (11 b iii)
- **F** — AVX is the most common technique today for implementing ILP. *AVX implements SIMD or subword parallelism.* (Q7 i)
- **F** — SIMD is often used in applications with task-level parallelism. *SIMD is used when there is data-level parallelism.* (8 a v)
- **F** — SIMD corresponds to the notion of thread-level parallelism. *It corresponds to data-level parallelism.* (11 b ii)
- **F** — Data-level parallelism means that one instruction operates on one data item. *Each instruction operates on several data items.* (12 a i)
- **F** — Software threads are typically used when implementing SIMD instructions. *Software threads give concurrency in an operating system and are not related to SIMD.* (13 b iv)
- **F** — Multicore is the only really important parallel construct that can give great speedup. *SIMD instructions can also greatly affect performance.* (Q8 iii)
- Classify: multimedia extension → **SIMD**; data-level parallelism → **SIMD**. (Q10 b iii, iv)
- Explain: an add that adds 4 different 16-bit words in one cycle is **subword parallelism / SIMD / data-level parallelism**. (Q11 a)
- Calculation: `vaddpd` with 256-bit registers does 256 / 64 = **4** double-precision operations in parallel. (7 c, in `module6_misc.c`)

## SISD, MISD, MIMD (the other three of Flynn's four)

- **F** — SISD is very unusual and there have been extremely few SISD processors. *SISD is very common: all single-core processors with one instruction stream on one data stream.* (7 b i)
- **F** — MISD is a useful technique for ILP. *MISD is not used much, and has nothing to do with ILP.* (5 b ii)
- **T** — A good example of MIMD is a modern multicore processor. (Q12 1)
- **F** — The main benefit of multicore is data-level parallelism (all cores run the same instruction sequence on different data). *A multicore uses task-level parallelism; each core has its own instruction stream.* (Q7 iv)
- **F** — Data-level parallelism and MIMD have much in common, in that one stream of instructions operates on multiple data. *MIMD has multiple instruction streams.* (Q13 c 2)
- Classify: multicore → **MIMD**. (Q10 b i)

## ILP, VLIW and superscalar (also out-of-order and register renaming)

The one thing to remember: both VLIW and superscalar are **multiple issue**, and multiple issue is **ILP**.
**VLIW = static = the compiler schedules at compile time. Superscalar = dynamic = the hardware schedules while running.**
Register renaming belongs to **out-of-order** processors.

- **T** — A processor that supports VLIW is designed to utilize ILP. (3 a iii)
- **T** — VLIW is a form of ILP. (12 a iv)
- **T** — Multiple issue is one form of ILP. (7 b iii)
- **T** — VLIW processors require that instruction scheduling is done statically, at compile time. (10 b v)
- **T** — ILP is available in almost all modern processors for laptops and workstations. (8 a iv)
- **T** — Unrolling a loop can improve performance by better utilizing ILP. (5 b iv)
- **T** — Register renaming is a way to solve hazards due to out-of-order execution. (3 a iv)
- **T** — An out-of-order processor can rearrange the order of instructions dynamically, as long as the result does not change. (Q13 c 1)
- **F** — The benefit of VLIW is that all scheduling is done automatically by the processor in hardware. *VLIW uses static scheduling by the compiler at compile time.* (1 a i)
- **F** — VLIW performs instruction scheduling dynamically in hardware. *The compiler schedules statically, before the program runs.* (6 a v)
- **F** — Superscalar processors are extremely energy efficient because all scheduling is done by the compiler. *That describes VLIW. A superscalar processor schedules in hardware.* (2 b iii)
- **F** — ILP is not used in modern processors because it gives no performance gains anymore. *ILP is very common and gives good performance gain.* (2 b ii)
- **F** — Register renaming is another name for forwarding. *Register renaming solves hazards due to out-of-order execution.* (4 b ii)
- **F** — Register renaming is a way to solve control hazards in single-cycle processors. *Register renaming is used in out-of-order processors.* (13 b i)
- **F** — All high-performance commercial processors have between 4 and 6 pipeline stages. *Many more are common, e.g. 14 stages for an Intel Core i5.* (4 b iii)
- **F** — A semaphore enables explicit ILP. *A semaphore is for synchronization in concurrent programming.* (1 a iii)
- **F** — Hyper-threading is another name for VLIW. *Hyper-threading is SMT; VLIW is a form of multiple issue.* (8 a iii)
- **F** — MapReduce is an efficient way for handling VLIW. *MapReduce is a programming model for distributed computing; VLIW is a way to get ILP.* (11 b v)
- Classify: VLIW → **ILP**; multiple issue → **ILP**. (Q10 b ii, vi)
- Explain: main difference VLIW versus superscalar — **superscalar schedules dynamically (in hardware), VLIW needs the compiler to schedule statically at compile time.** (Q10 a)
- Explain: `lw $t1,16($t0)` then `sub $t0,$t3,10` — a **write after read (WAR)** dependency on `$t0`. (Q13 b)
- Calculation: CPI of a loop on a 2-slot static multiple-issue processor = 18/27 = **2/3**. (Q11 c, in `module6_misc.c`)

## Hardware multithreading (also SMT and hyper-threading)

The one thing to remember: hardware multithreading is done **by the hardware, inside one core**, to **hide latencies**
(waiting time) and avoid stalls. **Hyper-threading = Intel's name for SMT.** It is not the operating system's
scheduling, not VLIW, and not about fetching several instructions at once.

- **T** — Some of the main motivations behind hardware multithreading are to hide latencies and to avoid stalls. (6 a iv)
- **T** — Hardware multithreading is typically used for hiding latencies. (11 b i)
- **T** — One of the main uses of hardware multithreading is to increase throughput by switching threads when the processor needs to wait, e.g. on memory requests. (Q13 c 3)
- **T** — GPUs use hardware multithreading as a technique for hiding latencies. (2 b iv)
- **T** — Hardware multithreading results in concurrency and not in parallelism. (12 a iii)
- **T** — Hyper-threading is another name for simultaneous multithreading. (5 b v)
- **T** — Hyper-threading is Intel's name for SMT. (13 b ii)
- **T** — Intel's Hyper-threading technology is making use of hardware multithreading. (Q8 iv)
- **T** (false also accepted with a correct argument) — One important component of SMT is fine-grained multithreading. (9 b v)
- **F** — Hardware multithreading enables fine-grained fetching of multiple instructions in parallel. *The processor switches between hardware threads; it does not fetch multiple instructions. It hides latencies.* (1 a iv)
- **F** — Hardware multithreading is a concurrency mechanism inside a process of an operating system. *Hardware threads are scheduled by the hardware, not by the operating system.* (8 a ii)
- **F** — Hyper-threading is Intel's approach for scheduling software threads efficiently within the operating system. *It is Intel's name and implementation of SMT.* (3 a v)
- **F** — Hyper-threading is another name for VLIW. *Hyper-threading is SMT.* (8 a iii)
- **F** — One of the main problems of hardware multithreading is cache coherence. *Not related. Cache coherence appears in a multicore with separate caches.* (4 b v)
- **F** — AVX is a form/example of hardware (multi)threading. *AVX is SIMD.* (5 b i, 12 a ii)
- Classify: hardware multithreading → **Hiding latencies**. (Q10 b v)

## Threads, semaphores and mutexes (also concurrency versus parallelism)

The one thing to remember: software threads are scheduled by the **operating system**. Semaphores and mutexes are
**software** tools for **synchronizing** threads and protecting shared resources. They have nothing to do with ILP
or with caches. A **mutex** is not a **multiplexer**.

- **T** — In modern operating systems, it is common that software threads are scheduled on different cores. (1 a ii)
- **T** — Software threads are scheduled by the operating system. (10 b ii)
- **T** — A concurrent program can potentially be scheduled to run in parallel by an operating system on a multicore processor. (6 a i)
- **T** — Linux on a processor with just one core is a good example of concurrency without parallelism. (9 b i)
- **T** — Semaphores are useful when developing concurrent programs. (5 b iii)
- **T** — Semaphores can be used to synchronize software threads. (13 b v)
- **T** — In concurrent programs, semaphores can be used to make sure that shared resources are accessed in a correct way. (Q7 iii)
- **T** (false also accepted with a valid motivation) — A mutex is a binary semaphore that is useful in concurrent programming. (2 b v)
- **T** — With a semaphore used as a mutex, the programmer locks the mutex before a critical section and unlocks it after, which gives controlled access to shared resources. (Q12 3)
- **F** — A semaphore is a software construct that enables explicit ILP. *It is used for synchronization in concurrent programming.* (1 a iii)
- **F** — A semaphore is a clever way to avoid the cache coherence problem. *Semaphores handle shared resources in concurrent programs; cache coherence is a problem in multicore systems with caches.* (10 b iii)
- **F** — A multiplexer can be used to stop several threads from accessing the same resource. *A mutex does that. A multiplexer is used in digital design.* (9 b ii)
- **F** — Software threads are typically used when implementing SIMD instructions. *Not related.* (13 b iv)
- **F** — Hardware multithreading is a concurrency mechanism inside an operating system process. *Hardware threads are scheduled by the hardware.* (8 a ii)

## Scaling, speedup and Moore's law

- **T** — For strong scaling, the speedup is estimated by keeping the program size fixed. (4 b i)
- **F** — Weak scaling means the experiment was done on few computers with weak processing power. *Weak scaling means the data size is also increased when the number of cores is increased.* (3 a i)
- **F** — Superlinear speedups are very common in practice, especially on superscalar processors. *Very uncommon; such a result often means the measurement is wrong. "Super" in superscalar is not related.* (6 a iii)
- **T** — Moore's law means that the number of transistors doubles approximately every second year. (7 b ii)
- **F** — Moore's law says that your computer gets twice as hot every second year. *It says the number of transistors on an integrated circuit doubles every 18-24 months.* (Q7 ii)
- **F** — Moore's law stopped working around 2006 because the processors got too hot. *Moore's law still holds (the key's words). It is about the number of transistors, not about power.* (Q8 i)
- Calculations with Amdahl's law (speedup, maximal speedup, cores needed, relative versus true speedup) are in `module6_amdahl.c`.

## Multicore, cache coherence and false sharing

The one thing to remember: **cache coherence problem** = cores with separate caches see **different values for the
same address**. **False sharing** = threads use **different addresses in the same cache block**, so they invalidate
each other's cache without sharing data. Neither has anything to do with virtual memory pages or with semaphores.

- **T** — Shared memory multiprocessors (SMP) are today one of the most common types of multicore processors. (1 a v)
- **T** — A major challenge for shared memory multicore processors is the cache coherence problem: different cores can see different values for the same memory address. (Q8 v)
- **T** — One way of handling the cache coherency problem is to use the snooping protocol. (11 b iv)
- **T** — False sharing is when different cores invalidate the same part of the cache without actually accessing the same data. (9 b iv)
- **F** — False sharing means that two software threads share the same page in virtual memory. *It means two threads use the same cache line, which invalidates the cache. Nothing to do with virtual memory.* (12 a v)
- **F** — Two cores frequently read or write the SAME address and get inconsistent L1 caches; this is called false sharing. *That is cache coherence. For false sharing the cores would write to the same cache block but not the exact same address.* (Q12 4)
- **F** — Two cores read the same address and one always gets the data faster; this is a cache coherence problem. *Cache coherence is about different values for the same address in different caches, e.g. after a write. Reading alone is not the problem.* (Q13 c 4)
- **F** — One of the main problems of hardware multithreading is cache coherence. *Cache coherence appears in a multicore processor with separate caches.* (4 b v)
- **F** — A semaphore is a clever way to avoid the cache coherence problem. *Not related.* (10 b iii)

## MapReduce

- **T** — MapReduce is a programming model that is commonly used for parallel computations on cluster systems. (9 b iii)
- **T** — MapReduce can be based on message passing techniques and is today used extensively in warehouse-scale computers. (Q12 2)
- **F** — MapReduce is an efficient way for handling VLIW. *MapReduce is a programming model, typically used in distributed computing. VLIW is a way to achieve ILP.* (11 b v)
