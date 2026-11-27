# Module 5: Memory Hierarchy — the questions that cannot be run

This file holds the explain-in-words, true/false and multiple-choice questions of Q5.
The calculations are in `module5_cache_fields.c` (field sizes, sets, valid bits, capacity)
and `module5_cache_sim.c` (hit and miss rates, the given-cache-state questions).

How to read it:

- Questions are grouped **by exam sitting**. Item numbers are the sittings in date order
  (1 = 2016-06-09 ... 13 = 2020-06-05), the same numbers as in the C files.
- Questions that exist only as a screenshot are at the end, named after the PNG file
  (for example `extra_Q5_10`).
- "Key" is the examiner's answer. The motivation is the examiner's, put in plain words.
- A sitting that is not listed has no theory question; all of it is in the C files.

---

## Words you need first

| Word | Plain meaning |
|---|---|
| **Cache** | A small, fast memory between the processor and the main memory. It keeps copies of blocks that were used recently. |
| **Block** | The unit that is copied into the cache. On a miss the **whole block** is loaded, never a single byte. |
| **Set** (row, index) | One row of the cache. The set field of the address says which row to look in. |
| **Way** | One place for a block inside a set. Direct mapped = 1 way. N-way set associative = N ways in every set. |
| **Valid bit** | One bit per block. 1 = the block holds real data. 0 = ignore whatever is there. |
| **Tag** | The top bits of the address, stored next to the block. It says *which* block is stored in this place. |
| **Hit / miss** | Hit = the set has a block with valid = 1 **and** the same tag. Otherwise it is a miss. |
| **Temporal locality** | An address that was used recently is likely to be used again soon. (Loops: the same instructions run again.) |
| **Spatial locality** | Addresses close to a used address are likely to be used soon. (The next instruction; the next array element.) A block with more than one word uses this. |
| **LRU** | Least Recently Used. On a miss in a full set, throw out the block that was used the longest time ago. Only needed when a set has more than one way. |
| **Write-through** | When writing, the data is written to the cache **and** to main memory at the same time. |
| **Write-back** | When writing, only the cache is written and a dirty bit is set. Main memory is updated later, when the block is thrown out. |
| **L1, L2, L3** | Cache levels. L1 is smallest, fastest and closest to the processor. L2 and L3 are larger and slower. |
| **Virtual memory** | Each process gets its own large address space. Pages are translated to physical memory by a page table; pages that do not fit are kept on disk. |
| **SRAM / DRAM** | SRAM: fast, expensive, used for caches. DRAM: stores bits in capacitors, cheaper, used for main memory, must be refreshed. |

### How the exam key decides "which locality?"

Look at the **hits** only:

- A hit on an address that was used **before** → temporal locality.
- A hit on an address that is used for the **first time** (a neighbour loaded its block) → spatial locality.
- No hits at all → neither.

`module5_cache_sim.c` counts both kinds of hit, so it prints the locality answer for every item below.

| Item | File | Instruction cache | Data cache |
|---|---|---|---|
| 4 (b) | 2017-06-08 Q5.pdf | both | spatial only |
| 6 (b) | 2018-03-15 Q5.pdf | both | (no data cache) |
| 7 (b) | 2018-06-05 Q5.pdf | spatial only | spatial only |
| 8 (b) | 2019-01-12-riscv Q5.pdf | both | spatial only |
| 10 (b) | 2019-06-05-riscv Q5.pdf | spatial only | (not asked) |
| 11 (b) | 2020-01-13-riscv Q5.pdf | spatial only | (not asked) |
| 13 (b) | 2020-06-05-riscv Q5.pdf | both | spatial only |
| extra_Q5_7 (b) | Q5-Q7.png | both | neither |

Rule of thumb: a **loop** gives the instruction cache temporal locality. A straight run of
instructions that is executed once can only show spatial locality. A data pointer that moves
forward and never comes back can only show spatial locality, and only if two accesses land in the same block.

---

## Item 1 — 2016-06-09 Q5.pdf

### Item 1 (c)

**Question.** Separate data and instruction caches. The processor executes

```
lw $t0,0($t1)        RISC-V: lw t0,0(t1)
```

We know nothing about the state of the caches. Can this give an instruction cache miss, a data cache miss, or both? (At most 3 sentences.)

**Key: both.**

- The instruction itself must first be **fetched**. If the block that holds the `lw` is not in the instruction cache, that is an instruction cache miss.
- Then the `lw` **reads data** at the address in `t1`. If that block is not in the data cache, that is a data cache miss.

Points: 1 for "both", 1 for the motivation.

---

## Item 3 — 2017-03-16 Q5.pdf

### Item 3 (a) ii

**Question.** Direct mapped data cache, 1024 bytes, 128 sets. A load byte instruction misses. Three different parts of the cache are updated because of the miss. Which?

**Key.** For the selected set:

1. the **valid bit** is set to 1,
2. the **tag** is updated,
3. the **cache block** (the data) is updated.

Part i (8 bytes are loaded, because a whole block is always fetched) is computed in `module5_cache_fields.c`, `item3a`.
In `module5_cache_sim.c` you can see these three updates in step 4 of `cacheAccess`.

Points: 2 for all three parts, 1 if only two are given.

---

## Item 5 — 2018-01-15 Q5.pdf

### Item 5 (b) and (c)

These are run in `module5_cache_sim.c` (`item5b`, `item5c`). The exam wants the answer **in words**, so here is how to write it.

Cache: 2-way, LRU, 8192 bytes, 256 sets → block size 8192 / (256 × 2) = 16 bytes → 4 byte offset bits, 8 set bits, 20 tag bits.

**(b)** Instruction at `0x40012460`. Hit or miss? (At most 5 sentences.)

**Key: hit.** The address has tag `0x40012`, set `0x46`, byte offset `0x0`. In set `0x46`, way 1 is valid and has the same tag.

Points: 1 for "hit", 1 for the valid-bit argument, 1 for the tag argument.

**(c)** Same starting state. Instruction at `0x40012474`. Does the cache state change?

**Key: yes.** The address has tag `0x40012` and set `0x47`. It is a **miss**: way 1 of set `0x47` is not valid, and way 2 is valid but has another tag (`0x30010`). The block is put in way 1:

- the valid bit of set `0x47`, way 1 is set to 1,
- the tag is written (it already had the value `0x40012`, so it is updated to the same value),
- the block data is updated.

Trap: way 1 already has the *right tag*, but its valid bit is 0, so the tag means nothing. It is still a miss.

---

## Item 6 — 2018-03-15 Q5.pdf

### Item 6 (c)

**Question.** Which are the two most important properties of virtual memories?

**Key.**

1. To give the **illusion of a large memory**.
2. To give **memory protection** between concurrently running programs (processes).

One point for each.

### Item 6 (d)

**Question.** Which one alternative is true?

| | Alternative | |
|---|---|---|
| i | Typically, an L1 cache has larger capacity than an L2 cache. | false: L1 is the smaller one |
| ii | Typically, an L2 cache has shorter access times than an L1 cache. | false: L2 is larger and therefore slower |
| iii | Typically, an L1 cache is located closer to the processor than an L2 cache. | **true** |
| iv | Processors never have more than two levels of caches. | false: many have an L3 cache |

**Key: alternative iii.** (The exam asks for no motivation. The reasons in the table are from lecture 11, not from the key.)

---

## Extra questions (only as screenshots)

## extra_Q5_10 — Q5-Q10.png

### extra_Q5_10 (c)

**Question.** We write to the main memory and to the cache at the same time. Which kind of write policy is this?

**Key: a write-through policy.**

---

## extra_Q5_11 — Q5-Q11.png

### extra_Q5_11 (b), second bullet

**Question.** A direct mapped instruction cache (256 sets, 16-byte blocks) holds 4096 bytes of data. What other information must be stored in the cache, and how many bits per set?

**Key.** Besides the data: a **valid bit** (1 bit) and the **tag** (32 − 8 − 4 = 20 bits).
The sizes are computed in `module5_cache_fields.c`, `extra_Q5_11b_sizes`.

### extra_Q5_11 (c)

**Question.** In modern operating systems each process gets the illusion of one large memory space that can be larger than the physical memory. What is it called, and why is it useful besides the illusion of a big memory?

**Key.** It is called **virtual memory**. The other benefit: memory is **protected** from other concurrently running processes in the operating system.

---

## extra_Q5_12 — Q5-Q12.png

### extra_Q5_12 (b) — true / false / don't know (wrong answers give minus points)

| # | Statement | Key | Motivation |
|---|---|---|---|
| 1 | A common replacement policy for direct mapped caches is LRU, since a set does not always have to be replaced directly, even if the validity bit is 1. | **False** | A direct mapped cache needs no replacement policy. There is only one place per set, so if the set is used it must always be replaced. |
| 2 | One benefit of virtual memory is that each process can have its own virtual memory space, protected from other concurrently running processes. | **True** | |
| 3 | Direct mapped cache, 16-byte blocks, 256 blocks. A load byte reads `0xff20215e`, then another load byte reads `0x10005150`. The second load is a cache miss. | **True** | Both addresses use the same set (`0x15`) but have different tags. Run in `module5_cache_sim.c`, `extra_Q5_12b3`. |
| 4 | A modern processor such as the Intel Core-I7 has only one large cache, because one large cache is typically faster than several small caches. | **False** | A modern processor has several caches in the memory hierarchy (L1, L2 and L3). |

---

## extra_Q5_13 — Q5-Q13.png

### extra_Q5_13 (c) — true / false / don't know

| # | Statement | Key | Motivation |
|---|---|---|---|
| 1 | The main reason for a multi-level cache is to have faster, smaller caches closer to the processor and larger but slower caches further down in the memory hierarchy. | **True** | |
| 2 | A write-back policy is a simple way to handle memory writes, where data is always written back to main memory directly when data is written to the cache. | **False** | That describes a **write-through** policy. |
| 3 | Virtual memory with page size 1024 bytes, 32-bit virtual address, 28-bit physical address. A page table translates 20-bit virtual page numbers into 16-bit physical page numbers. | **False** | The page offset is 10 bits (2^10 = 1024). So 32 − 10 = **22**-bit virtual page numbers are translated into 28 − 10 = **18**-bit physical page numbers. Computed in `module5_cache_fields.c`, `extra_Q5_13c3`. |
| 4 | One major difference between DRAM and SRAM memories is that DRAM memories need to be periodically refreshed, whereas SRAM memories do not need any refresh. | *(not visible)* | **The screenshot is cut off before the examiner's answer.** From lecture 11 ("DRAM: memory stored in capacitors – need to be refreshed"; SRAM is static) the statement is **true**, but this is not the examiner's printed key. |
