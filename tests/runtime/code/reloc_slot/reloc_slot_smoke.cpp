#include <chrono>
#include <cstdio>
#include <set>
#include <thread>

extern "C" void* stabilizer_get_relocation_table(void* symbol);
extern "C" void* stabilizer_get_active_code_location(void* symbol);

static volatile int g_value = 0;

static __attribute__((noinline)) int* slot_probe() {
    return const_cast<int*>(&g_value);
}

int main() {
    void* symbol = reinterpret_cast<void*>(&slot_probe);
    std::set<void*> tables;
    std::set<void*> locations;

    for (int i = 0; i < 6; ++i) {
        (void)*slot_probe();

        if (void* table = stabilizer_get_relocation_table(symbol)) {
            tables.insert(table);
        }
        if (void* body = stabilizer_get_active_code_location(symbol)) {
            locations.insert(body);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }

    if (locations.size() < 2) {
        std::fprintf(stderr,
                     "Function never relocated: only %zu unique bodies\n",
                     locations.size());
        return 1;
    }

    if (tables.size() < 2) {
        std::fprintf(stderr,
                     "Relocation table pointer never updated (unique=%zu)\n",
                     tables.size());
        return 1;
    }

    std::puts("Relocation slot smoke test passed.");
    return 0;
}

