#include <chrono>
#include <cstdio>
#include <cstdint>
#include <set>
#include <thread>

static __attribute__((noinline)) uintptr_t sample_stack() {
    return reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
}

int main() {
    constexpr int kSamples = 8;
    constexpr auto kDelay = std::chrono::milliseconds(600);

    std::set<uintptr_t> values;
    for (int i = 0; i < kSamples; ++i) {
        auto value = sample_stack();
        values.insert(value);
        std::printf("STACK_SAMPLE[%d]=%p\n", i,
                    reinterpret_cast<void*>(value));
        std::this_thread::sleep_for(kDelay);
    }

    if (values.size() < 2) {
        std::fprintf(stderr,
                     "Stack randomization inactive: only %zu unique values\n",
                     values.size());
        return 1;
    }

    std::puts("Stack randomization smoke test passed.");
    return 0;
}
