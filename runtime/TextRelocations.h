#if !defined(RUNTIME_TEXTRELOCATIONS_H)
#define RUNTIME_TEXTRELOCATIONS_H

#include <set>

struct Function;

/**
 * \brief Pre-compute text relocation entries for each Function.
 *
 * When code randomization is enabled, Stabilizer copies machine code bytes to a
 * new location. Any remaining pc-relative references to data outside the copied
 * allocation (e.g., constant pools in .rodata) must be patched so they continue
 * to point at the original data from the new location.
 *
 * This routine scans the program's ELF relocation sections (requires linking
 * with `--emit-relocs`) and assigns relevant relocations to each registered
 * Function.
 */
bool stabilizer_init_text_relocations(const std::set<Function*>& functions);

#endif

