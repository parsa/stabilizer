#include "TextRelocations.h"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <elf.h>

#if defined(__linux__)
#include <link.h>
#endif

#include "Debug.h"
#include "Function.h"

using std::set;
using std::vector;

static uintptr_t get_main_load_bias() {
#if defined(__linux__)
    struct BiasCtx {
        uintptr_t bias = 0;
        bool found = false;
    } ctx;

    dl_iterate_phdr(
        [](struct dl_phdr_info* info, size_t, void* data) -> int {
            BiasCtx* c = (BiasCtx*)data;
            // The main executable typically has an empty name.
            if(info->dlpi_name == NULL || info->dlpi_name[0] == '\0') {
                c->bias = (uintptr_t)info->dlpi_addr;
                c->found = true;
                return 1; // stop
            }
            return 0;
        },
        &ctx
    );

    return ctx.found ? ctx.bias : 0;
#else
    return 0;
#endif
}

bool stabilizer_init_text_relocations(const set<Function*>& functions) {
#if !defined(__x86_64__)
    (void)functions;
    return true;
#else
    if(functions.empty()) {
        return true;
    }

    // Build a sorted vector of functions by base address to make range checks fast.
    vector<Function*> sorted;
    sorted.reserve(functions.size());
    for(Function* f : functions) {
        sorted.push_back(f);
    }
    std::sort(sorted.begin(), sorted.end(), [](Function* a, Function* b) {
        return (uintptr_t)a->getCodeBase() < (uintptr_t)b->getCodeBase();
    });

    int fd = open("/proc/self/exe", O_RDONLY);
    if(fd < 0) {
        DEBUG("Unable to open /proc/self/exe: %s", strerror(errno));
        return false;
    }

    struct stat st;
    if(fstat(fd, &st) != 0) {
        DEBUG("Unable to stat /proc/self/exe: %s", strerror(errno));
        close(fd);
        return false;
    }

    void* file = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if(file == MAP_FAILED) {
        DEBUG("Unable to mmap /proc/self/exe: %s", strerror(errno));
        return false;
    }

    uint8_t* bytes = (uint8_t*)file;
    if((size_t)st.st_size < sizeof(Elf64_Ehdr)) {
        munmap(file, st.st_size);
        return false;
    }

    Elf64_Ehdr* eh = (Elf64_Ehdr*)bytes;
    if(!(eh->e_ident[EI_MAG0] == ELFMAG0 &&
         eh->e_ident[EI_MAG1] == ELFMAG1 &&
         eh->e_ident[EI_MAG2] == ELFMAG2 &&
         eh->e_ident[EI_MAG3] == ELFMAG3)) {
        munmap(file, st.st_size);
        return false;
    }
    if(eh->e_ident[EI_CLASS] != ELFCLASS64) {
        munmap(file, st.st_size);
        return false;
    }

    if(eh->e_shoff == 0 || eh->e_shentsize != sizeof(Elf64_Shdr) || eh->e_shnum == 0) {
        munmap(file, st.st_size);
        return false;
    }
    if((size_t)eh->e_shoff + (size_t)eh->e_shnum * sizeof(Elf64_Shdr) > (size_t)st.st_size) {
        munmap(file, st.st_size);
        return false;
    }

    Elf64_Shdr* sh = (Elf64_Shdr*)(bytes + eh->e_shoff);

    uintptr_t load_bias = 0;
    if(eh->e_type == ET_DYN) {
        load_bias = get_main_load_bias();
    }

    auto find_function_for_addr = [&](uintptr_t P) -> Function* {
        // upper_bound: first function with base > P
        auto it = std::upper_bound(
            sorted.begin(),
            sorted.end(),
            P,
            [](uintptr_t addr, Function* f) {
                return addr < (uintptr_t)f->getCodeBase();
            }
        );
        if(it == sorted.begin()) {
            return NULL;
        }
        Function* cand = *(it - 1);
        uintptr_t base = (uintptr_t)cand->getCodeBase();
        uintptr_t end = base + cand->getCodeSize();
        if(P >= base && P < end) {
            return cand;
        }
        return NULL;
    };

    size_t exec_rela_sections = 0;
    size_t exec_rela_entries = 0;
    size_t supported_relocs_added = 0;

    // Scan all SHT_RELA sections that target executable sections.
    for(uint16_t i = 0; i < eh->e_shnum; i++) {
        if(sh[i].sh_type != SHT_RELA) {
            continue;
        }
        if(sh[i].sh_info >= eh->e_shnum) {
            continue;
        }

        const Elf64_Shdr& target = sh[sh[i].sh_info];
        if((target.sh_flags & SHF_EXECINSTR) == 0) {
            continue;
        }

        exec_rela_sections++;

        // Bounds check relocation section contents.
        if(sh[i].sh_offset + sh[i].sh_size > (size_t)st.st_size) {
            continue;
        }

        const Elf64_Rela* relas = (const Elf64_Rela*)(bytes + sh[i].sh_offset);
        size_t n = sh[i].sh_size / sizeof(Elf64_Rela);
        exec_rela_entries += n;

        // Optional filter: only keep relocation types we know how to patch.
        for(size_t r = 0; r < n; r++) {
            uint32_t type = ELF64_R_TYPE(relas[r].r_info);

            // PC-relative relocation types that require fixups when code moves.
            if(type != R_X86_64_PC32 &&
               type != R_X86_64_PLT32 &&
               type != R_X86_64_GOTPCREL &&
               type != R_X86_64_GOTPCRELX &&
               type != R_X86_64_REX_GOTPCRELX) {
                continue;
            }

            // In final linked ELF binaries (ET_EXEC/ET_DYN), r_offset is a virtual
            // address. For PIE (ET_DYN), add the load bias.
            uintptr_t P = load_bias + (uintptr_t)relas[r].r_offset;
            Function* f = find_function_for_addr(P);
            if(f == NULL) {
                continue;
            }

            size_t off = (size_t)(P - (uintptr_t)f->getCodeBase());
            f->addTextReloc(off, type, relas[r].r_addend);
            supported_relocs_added++;
        }
    }

    munmap(file, st.st_size);

    // Diagnostics: these are best-effort; failure to load relocations should not
    // crash programs that don't need them.
    if(exec_rela_sections == 0) {
        DEBUG("No executable RELA sections found in /proc/self/exe (missing -Wl,--emit-relocs?)");
        return false;
    } else if(supported_relocs_added == 0) {
        DEBUG("Found %zu executable RELA sections (%zu entries) but recorded 0 supported relocations", exec_rela_sections, exec_rela_entries);
    }

    return true;
#endif
}

