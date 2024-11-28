#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <atomic>

struct Config {
    int num_cpu = 1;
    std::string scheduler = "fcfs";
    int quantum_cycles = 1;
    int batch_process_freq = 1;
    int min_ins = 1;
    int max_ins = 1;
    int delays_per_exec = 0;
    int max_overall_mem;
    int mem_per_frame;
    int mem_per_proc;
};

extern Config config;

extern std::atomic<int> activeCores;

template <typename T1, typename T2, typename T3>
auto clamp(const T1& v, const T2& lo, const T3& hi) -> typename std::common_type<T1, T2, T3>::type {
    using CommonType = typename std::common_type<T1, T2, T3>::type;
    return (v < static_cast<CommonType>(lo)) ? static_cast<CommonType>(lo)
        : (static_cast<CommonType>(hi) < v) ? static_cast<CommonType>(hi)
        : static_cast<CommonType>(v);
}

#endif // CONFIG_H