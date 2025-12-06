#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <thread>

static __attribute__((noinline)) void* emit_allocation() {
    void* ptr = std::malloc(128);
    return ptr;
}

int main() {
    constexpr int kSamples = 12;
    constexpr auto kDelay = std::chrono::milliseconds(200);

    std::set<uintptr_t> values;
    for (int i = 0; i < kSamples; ++i) {
        void* ptr = emit_allocation();
        values.insert(reinterpret_cast<uintptr_t>(ptr));
        std::printf("HEAP_SAMPLE[%d]=%p\n", i, ptr);
        std::free(ptr);
        std::this_thread::sleep_for(kDelay);
    }

    if (values.size() < 2) {
        std::fprintf(stderr,
                     "Heap randomization inactive: only %zu unique values\n",
                     values.size());
        return 1;
    }

    std::puts("Heap randomization smoke test passed.");
    return 0;
}
