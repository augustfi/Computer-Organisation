# Rules for this repo

- When running an item, always show the full standalone code for that item
  (`#include`, the item function, any globals or helper functions it uses, and `main`)
  and then its output.
- The exam assumes 32-bit RISC-V. Point out any value that is unknown on the exam
  (uninitialised variables, addresses not given in the question).
- To run one item, compile a copy with only that item enabled in `main()`; don't
  leave the repo's `main()` changed unless asked.
- C file: `gcc -Wno-pointer-to-int-cast module1_c_programming.c -o module1 && ./module1`
- Machine-code file: `gcc module1_machine_code.c -o machine && ./machine`
