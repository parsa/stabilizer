#include <chrono>
#include <cstdio>
#include <set>
#include <thread>

extern "C" void* stabilizer_get_active_code_location(void* symbol);

static __attribute__((noinline)) int code_probe(int value) {
    return value + 1;
}

int main() {
    void* symbol = reinterpret_cast<void*>(&code_probe);
    std::set<void*> locations;

    for (int i = 0; i < 6; ++i) {
        code_probe(i);
        if (void* loc = stabilizer_get_active_code_location(symbol)) {
            locations.insert(loc);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }

    if (locations.size() < 2) {
        std::fprintf(stderr,
                     "Code relocation inactive: only %zu unique bodies\n",
                     locations.size());
        return 1;
    }

    std::puts("Code relocation smoke test passed.");
    return 0;
}

