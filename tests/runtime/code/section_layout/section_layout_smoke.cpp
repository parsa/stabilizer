#include <cstdint>
#include <cstdio>

extern "C" void* stabilizer_get_active_code_location(void* symbol);
extern "C" size_t stabilizer_get_function_code_size(void* symbol);
extern "C" void* stabilizer_get_relocation_table(void* symbol);
extern "C" int stabilizer_is_table_adjacent(void* symbol);

static __attribute__((noinline)) int layout_probe(int x) {
    return x ^ 0x55;
}

int main() {
    void* symbol = reinterpret_cast<void*>(&layout_probe);
    layout_probe(0);  // ensure the function is registered

    size_t codeSize = stabilizer_get_function_code_size(symbol);
    void* body = stabilizer_get_active_code_location(symbol);
    void* table = stabilizer_get_relocation_table(symbol);
    int adjacent = stabilizer_is_table_adjacent(symbol);

    if (codeSize == 0 || body == nullptr || table == nullptr || !adjacent) {
        std::fprintf(stderr, "Section metadata missing (codeSize=%zu body=%p table=%p adjacent=%d)\n",
                     codeSize, body, table, adjacent);
        return 1;
    }

    uint8_t* expected = static_cast<uint8_t*>(body) + codeSize;
    if (expected != table) {
        std::fprintf(stderr,
                     "Relocation table not contiguous: expected %p got %p\n",
                     expected,
                     table);
        return 1;
    }

    std::puts("Section layout smoke test passed.");
    return 0;
}

