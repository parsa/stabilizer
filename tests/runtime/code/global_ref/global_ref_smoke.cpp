#include <chrono>
#include <cstdio>
#include <set>
#include <thread>

extern "C" void* stabilizer_get_active_code_location(void* symbol);

static volatile unsigned long g_counter = 0;

static __attribute__((noinline)) void touch_global() {
    ++g_counter;
}

int main() {
    constexpr int kIterations = 16;
    void* symbol = reinterpret_cast<void*>(&touch_global);
    std::set<void*> locations;

    for (int i = 0; i < kIterations; ++i) {
        touch_global();
        if (g_counter != static_cast<unsigned long>(i + 1)) {
            std::fprintf(stderr, "Global counter mismatch after relocation\n");
            return 1;
        }

        if (void* loc = stabilizer_get_active_code_location(symbol)) {
            locations.insert(loc);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(650));
    }

    if (locations.size() < 2) {
        std::fprintf(stderr,
                     "Function never relocated: only %zu unique bodies\n",
                     locations.size());
        return 1;
    }

    std::puts("Global reference smoke test passed.");
    return 0;
}

