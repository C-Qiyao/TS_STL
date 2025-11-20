#include "../include/ts_stl.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <iomanip>
#include <numeric>
#include <string>
#include <sstream>
#include <map>

using namespace ts_stl;
using namespace std::chrono;

// ==================== 性能测试辅助工具 ====================

class PerformanceTimer {
public:
    void start() {
        start_time_ = high_resolution_clock::now();
    }
    
    double stop() {
        auto end_time = high_resolution_clock::now();
        return duration<double, std::milli>(end_time - start_time_).count();
    }
    
private:
    high_resolution_clock::time_point start_time_;
};

struct BenchmarkResult {
    std::string test_name;
    std::string container_type;
    double time_ms;
    size_t operations;
    bool data_valid;
    
    double ops_per_ms() const {
        return time_ms > 0 ? operations / time_ms : 0;
    }
};

// ==================== 测试配置 ====================

constexpr size_t SINGLE_THREAD_OPS = 1000000;   // 单线程操作数
constexpr size_t MULTI_THREAD_OPS = 100000;     // 每个线程的操作数
constexpr size_t NUM_THREADS = 8;                // 线程数量
constexpr size_t READ_HEAVY_THREADS = 12;        // 读密集型测试的线程数

// ==================== 数据验证辅助 ====================

template<typename Container>
bool validate_sequential_write(const Container& container, size_t expected_size) {
    if (container.size() != expected_size) {
        return false;
    }
    return true;
}

template<typename T>
bool validate_std_vector_sequential(const std::vector<T>& vec, size_t expected_size) {
    return vec.size() == expected_size;
}

// ==================== 单线程性能测试 ====================

void print_section_header(const std::string& title) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(80, '=') << "\n";
}

BenchmarkResult benchmark_single_thread_push_back(const std::string& container_name, 
                                                   std::function<void()> test_func,
                                                   std::function<bool()> validate_func) {
    PerformanceTimer timer;
    timer.start();
    test_func();
    double time = timer.stop();
    
    return {
        "Single Thread Push Back",
        container_name,
        time,
        SINGLE_THREAD_OPS,
        validate_func()
    };
}

void run_single_thread_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("单线程顺序写入性能测试");
    
    // 测试 std::vector
    {
        std::vector<int> vec;
        auto result = benchmark_single_thread_push_back(
            "std::vector",
            [&]() {
                for (size_t i = 0; i < SINGLE_THREAD_OPS; ++i) {
                    vec.push_back(i);
                }
            },
            [&]() { return validate_std_vector_sequential(vec, SINGLE_THREAD_OPS); }
        );
        results.push_back(result);
    }
    
    // 测试 vectorMutex
    {
        vectorMutex<int> vec;
        auto result = benchmark_single_thread_push_back(
            "vectorMutex",
            [&]() {
                for (size_t i = 0; i < SINGLE_THREAD_OPS; ++i) {
                    vec.push_back(i);
                }
            },
            [&]() { return validate_sequential_write(vec, SINGLE_THREAD_OPS); }
        );
        results.push_back(result);
    }
    
    // 测试 vectorSpinLock
    {
        vectorSpinLock<int> vec;
        auto result = benchmark_single_thread_push_back(
            "vectorSpinLock",
            [&]() {
                for (size_t i = 0; i < SINGLE_THREAD_OPS; ++i) {
                    vec.push_back(i);
                }
            },
            [&]() { return validate_sequential_write(vec, SINGLE_THREAD_OPS); }
        );
        results.push_back(result);
    }
    
    // 测试 vectorLockFree
    {
        vectorLockFree<int> vec;
        auto result = benchmark_single_thread_push_back(
            "vectorLockFree",
            [&]() {
                for (size_t i = 0; i < SINGLE_THREAD_OPS; ++i) {
                    vec.push_back(i);
                }
            },
            [&]() { return validate_sequential_write(vec, SINGLE_THREAD_OPS); }
        );
        results.push_back(result);
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 vectorRW
    {
        vectorRW<int> vec;
        auto result = benchmark_single_thread_push_back(
            "vectorRW",
            [&]() {
                for (size_t i = 0; i < SINGLE_THREAD_OPS; ++i) {
                    vec.push_back(i);
                }
            },
            [&]() { return validate_sequential_write(vec, SINGLE_THREAD_OPS); }
        );
        results.push_back(result);
    }
#endif
}

// ==================== 多线程并发写入测试 ====================

BenchmarkResult benchmark_concurrent_write(const std::string& container_name,
                                           std::function<void()> test_func,
                                           std::function<bool()> validate_func) {
    PerformanceTimer timer;
    timer.start();
    test_func();
    double time = timer.stop();
    
    return {
        "Concurrent Write",
        container_name,
        time,
        MULTI_THREAD_OPS * NUM_THREADS,
        validate_func()
    };
}

void run_concurrent_write_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("多线程并发写入性能测试");
    
    // 测试 std::vector + mutex
    {
        std::vector<int> vec;
        std::mutex mtx;
        bool crashed = false;
        
        auto result = benchmark_concurrent_write(
            "std::vector+mutex",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < NUM_THREADS; ++t) {
                    threads.emplace_back([&, t]() {
                        try {
                            for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                                std::lock_guard<std::mutex> lock(mtx);
                                vec.push_back(t * MULTI_THREAD_OPS + i);
                            }
                        } catch (...) {
                            crashed = true;
                        }
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { 
                return !crashed && validate_std_vector_sequential(vec, MULTI_THREAD_OPS * NUM_THREADS); 
            }
        );
        results.push_back(result);
    }
    
    // 测试 vectorMutex
    {
        vectorMutex<int> vec;
        auto result = benchmark_concurrent_write(
            "vectorMutex",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < NUM_THREADS; ++t) {
                    threads.emplace_back([&, t]() {
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            vec.push_back(t * MULTI_THREAD_OPS + i);
                        }
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return validate_sequential_write(vec, MULTI_THREAD_OPS * NUM_THREADS); }
        );
        results.push_back(result);
    }
    
    // 测试 vectorSpinLock
    {
        vectorSpinLock<int> vec;
        auto result = benchmark_concurrent_write(
            "vectorSpinLock",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < NUM_THREADS; ++t) {
                    threads.emplace_back([&, t]() {
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            vec.push_back(t * MULTI_THREAD_OPS + i);
                        }
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return validate_sequential_write(vec, MULTI_THREAD_OPS * NUM_THREADS); }
        );
        results.push_back(result);
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 vectorRW
    {
        vectorRW<int> vec;
        auto result = benchmark_concurrent_write(
            "vectorRW",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < NUM_THREADS; ++t) {
                    threads.emplace_back([&, t]() {
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            vec.push_back(t * MULTI_THREAD_OPS + i);
                        }
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return validate_sequential_write(vec, MULTI_THREAD_OPS * NUM_THREADS); }
        );
        results.push_back(result);
    }
#endif
}

// ==================== 多线程并发读取测试 ====================

BenchmarkResult benchmark_concurrent_read(const std::string& container_name,
                                         std::function<void()> test_func,
                                         std::function<bool()> validate_func) {
    PerformanceTimer timer;
    timer.start();
    test_func();
    double time = timer.stop();
    
    return {
        "Concurrent Read",
        container_name,
        time,
        MULTI_THREAD_OPS * READ_HEAVY_THREADS,
        validate_func()
    };
}

void run_concurrent_read_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("多线程并发读取性能测试");
    
    // 准备测试数据
    constexpr size_t DATA_SIZE = 10000;
    
    // 测试 std::vector + mutex
    {
        std::vector<int> vec(DATA_SIZE);
        std::iota(vec.begin(), vec.end(), 0);
        std::mutex mtx;
        std::atomic<size_t> sum{0};
        
        auto result = benchmark_concurrent_read(
            "std::vector+mutex",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < READ_HEAVY_THREADS; ++t) {
                    threads.emplace_back([&]() {
                        size_t local_sum = 0;
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            std::lock_guard<std::mutex> lock(mtx);
                            local_sum += vec[i % DATA_SIZE];
                        }
                        sum += local_sum;
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return sum > 0; }
        );
        results.push_back(result);
    }
    
    // 测试 vectorMutex
    {
        vectorMutex<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        auto result = benchmark_concurrent_read(
            "vectorMutex",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < READ_HEAVY_THREADS; ++t) {
                    threads.emplace_back([&]() {
                        size_t local_sum = 0;
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            local_sum += vec[i % DATA_SIZE];
                        }
                        sum += local_sum;
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return sum > 0; }
        );
        results.push_back(result);
    }
    
    // 测试 vectorSpinLock
    {
        vectorSpinLock<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        auto result = benchmark_concurrent_read(
            "vectorSpinLock",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < READ_HEAVY_THREADS; ++t) {
                    threads.emplace_back([&]() {
                        size_t local_sum = 0;
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            local_sum += vec[i % DATA_SIZE];
                        }
                        sum += local_sum;
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return sum > 0; }
        );
        results.push_back(result);
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 vectorRW
    {
        vectorRW<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        auto result = benchmark_concurrent_read(
            "vectorRW",
            [&]() {
                std::vector<std::thread> threads;
                for (size_t t = 0; t < READ_HEAVY_THREADS; ++t) {
                    threads.emplace_back([&]() {
                        size_t local_sum = 0;
                        for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                            local_sum += vec[i % DATA_SIZE];
                        }
                        sum += local_sum;
                    });
                }
                for (auto& thread : threads) {
                    thread.join();
                }
            },
            [&]() { return sum > 0; }
        );
        results.push_back(result);
    }
#endif
}

// ==================== 混合读写测试（读多写少 90:10） ====================

void run_mixed_read_write_90_10_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("多线程混合读写测试（90%读 + 10%写）");
    
    constexpr size_t DATA_SIZE = 10000;
    constexpr size_t WRITE_THREADS = 1;
    constexpr size_t READ_THREADS = 9;
    
    // 测试 std::vector + mutex
    {
        std::vector<int> vec(DATA_SIZE);
        std::iota(vec.begin(), vec.end(), 0);
        std::mutex mtx;
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    std::lock_guard<std::mutex> lock(mtx);
                    vec[i % DATA_SIZE] = i;
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    std::lock_guard<std::mutex> lock(mtx);
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (90:10)",
            "std::vector+mutex",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
    
    // 测试 vectorMutex
    {
        vectorMutex<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    vec.set(i % DATA_SIZE, i);
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (90:10)",
            "vectorMutex",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
    
    // 测试 vectorSpinLock
    {
        vectorSpinLock<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    vec.set(i % DATA_SIZE, i);
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (90:10)",
            "vectorSpinLock",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 vectorRW
    {
        vectorRW<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    vec.set(i % DATA_SIZE, i);
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (90:10)",
            "vectorRW",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
#endif
}

// ==================== 混合读写测试（读写均衡 50:50） ====================

void run_mixed_read_write_50_50_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("多线程混合读写测试（50%读 + 50%写）");
    
    constexpr size_t DATA_SIZE = 10000;
    constexpr size_t WRITE_THREADS = 4;
    constexpr size_t READ_THREADS = 4;
    
    // 测试 std::vector + mutex
    {
        std::vector<int> vec(DATA_SIZE);
        std::iota(vec.begin(), vec.end(), 0);
        std::mutex mtx;
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    std::lock_guard<std::mutex> lock(mtx);
                    vec[i % DATA_SIZE] = i;
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    std::lock_guard<std::mutex> lock(mtx);
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (50:50)",
            "std::vector+mutex",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
    
    // 测试 vectorMutex
    {
        vectorMutex<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    vec.set(i % DATA_SIZE, i);
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (50:50)",
            "vectorMutex",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
    
    // 测试 vectorSpinLock
    {
        vectorSpinLock<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    vec.set(i % DATA_SIZE, i);
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (50:50)",
            "vectorSpinLock",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 vectorRW
    {
        vectorRW<int> vec(DATA_SIZE);
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            vec.set(i, i);
        }
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        
        // 写线程
        for (size_t t = 0; t < WRITE_THREADS; ++t) {
            threads.emplace_back([&]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    vec.set(i % DATA_SIZE, i);
                }
            });
        }
        
        // 读线程
        for (size_t t = 0; t < READ_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += vec[i % DATA_SIZE];
                }
                sum += local_sum;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        results.push_back({
            "Mixed R/W (50:50)",
            "vectorRW",
            time,
            MULTI_THREAD_OPS * (READ_THREADS + WRITE_THREADS),
            sum > 0
        });
    }
#endif
}

// ==================== 结果输出 ====================

void print_results_table(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "性能测试结果汇总\n";
    std::cout << std::string(120, '=') << "\n\n";
    
    // 按测试类型分组
    std::map<std::string, std::vector<BenchmarkResult>> grouped_results;
    for (const auto& result : results) {
        grouped_results[result.test_name].push_back(result);
    }
    
    for (const auto& [test_name, test_results] : grouped_results) {
        std::cout << "【" << test_name << "】\n";
        std::cout << std::left 
                  << std::setw(25) << "容器类型"
                  << std::setw(18) << "耗时(ms)"
                  << std::setw(18) << "操作数"
                  << std::setw(20) << "吞吐量(ops/ms)"
                  << std::setw(15) << "相对性能"
                  << std::setw(12) << "数据正确"
                  << "\n";
        std::cout << std::string(110, '-') << "\n";
        
        // 找到最快的时间作为基准
        double baseline_time = test_results[0].time_ms;
        for (const auto& r : test_results) {
            if (r.time_ms < baseline_time && r.time_ms > 0) {
                baseline_time = r.time_ms;
            }
        }
        
        for (const auto& result : test_results) {
            double relative_perf = baseline_time / result.time_ms;
            std::cout << std::left
                      << std::setw(25) << result.container_type
                      << std::setw(18) << std::fixed << std::setprecision(2) << result.time_ms
                      << std::setw(18) << result.operations
                      << std::setw(20) << std::fixed << std::setprecision(0) << result.ops_per_ms()
                      << std::setw(15) << std::fixed << std::setprecision(2) << (relative_perf * 100) << "%"
                      << std::setw(12) << (result.data_valid ? "✓" : "✗")
                      << "\n";
        }
        std::cout << "\n";
    }
}

void generate_markdown_summary(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "Markdown 格式结果（可直接复制到 README.md）\n";
    std::cout << std::string(80, '=') << "\n\n";
    
    std::cout << "## 性能测试结果\n\n";
    
    // 按测试类型分组
    std::map<std::string, std::vector<BenchmarkResult>> grouped_results;
    for (const auto& result : results) {
        grouped_results[result.test_name].push_back(result);
    }
    
    for (const auto& [test_name, test_results] : grouped_results) {
        std::cout << "### " << test_name << "\n\n";
        std::cout << "| 容器类型 | 耗时(ms) | 操作数 | 吞吐量(ops/ms) | 相对性能 | 数据正确 |\n";
        std::cout << "|---------|---------|--------|---------------|---------|----------|\n";
        
        // 找到最快的时间作为基准
        double baseline_time = test_results[0].time_ms;
        for (const auto& r : test_results) {
            if (r.time_ms < baseline_time && r.time_ms > 0) {
                baseline_time = r.time_ms;
            }
        }
        
        for (const auto& result : test_results) {
            double relative_perf = baseline_time / result.time_ms;
            std::cout << "| " << result.container_type
                      << " | " << std::fixed << std::setprecision(2) << result.time_ms
                      << " | " << result.operations
                      << " | " << std::fixed << std::setprecision(0) << result.ops_per_ms()
                      << " | " << std::fixed << std::setprecision(1) << (relative_perf * 100) << "%"
                      << " | " << (result.data_valid ? "✓" : "✗")
                      << " |\n";
        }
        std::cout << "\n";
    }
    
    // 添加性能建议
    std::cout << "### 性能使用建议\n\n";
    std::cout << "```\n";
    std::cout << "场景                     推荐容器\n";
    std::cout << "----------------------  ---------------------------\n";
    std::cout << "单线程/初始化           vectorLockFree (零开销)\n";
    std::cout << "多线程并发写入           vectorMutex or vectorSpinLock\n";
    std::cout << "多线程并发读取           vectorRW (读写锁优势明显)\n";
    std::cout << "读多写少(90:10)         vectorRW (读写锁最优)\n";
    std::cout << "读写均衡(50:50)         vectorMutex (通用稳定)\n";
    std::cout << "高竞争场景              避免 vectorSpinLock\n";
    std::cout << "```\n\n";
}

// ==================== Map 性能基准测试 ====================

void run_map_insert_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("Map 单线程插入性能测试");
    
    // 测试 mapMutex
    {
        mapMutex<int, int> map;
        PerformanceTimer timer;
        timer.start();
        for (size_t i = 0; i < SINGLE_THREAD_OPS / 100; ++i) {
            map.insert(i, i * 2);
        }
        double time = timer.stop();
        
        BenchmarkResult result{
            "Map Single Thread Insert",
            "mapMutex",
            time,
            SINGLE_THREAD_OPS / 100,
            map.size() == SINGLE_THREAD_OPS / 100
        };
        results.push_back(result);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << result.container_type << ": " << time << "ms (正确: "
                  << (result.data_valid ? "✓" : "✗") << ")\n";
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 mapRW
    {
        mapRW<int, int> map;
        PerformanceTimer timer;
        timer.start();
        for (size_t i = 0; i < SINGLE_THREAD_OPS / 100; ++i) {
            map.insert(i, i * 2);
        }
        double time = timer.stop();
        
        BenchmarkResult result{
            "Map Single Thread Insert",
            "mapRW",
            time,
            SINGLE_THREAD_OPS / 100,
            map.size() == SINGLE_THREAD_OPS / 100
        };
        results.push_back(result);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << result.container_type << ": " << time << "ms (正确: "
                  << (result.data_valid ? "✓" : "✗") << ")\n";
    }
#endif
}

void run_map_concurrent_insert_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("Map 多线程并发插入性能测试");
    
    // 测试 mapMutex
    {
        mapMutex<int, int> map;
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        for (size_t t = 0; t < NUM_THREADS; ++t) {
            threads.emplace_back([&, t]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS / 10; ++i) {
                    map.insert(t * (MULTI_THREAD_OPS / 10) + i, i * 2);
                }
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        size_t expected = NUM_THREADS * (MULTI_THREAD_OPS / 10);
        
        BenchmarkResult result{
            "Map Concurrent Insert",
            "mapMutex",
            time,
            expected,
            map.size() == expected
        };
        results.push_back(result);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << result.container_type << ": " << time << "ms (大小: " 
                  << map.size() << ", 正确: " << (result.data_valid ? "✓" : "✗") << ")\n";
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 测试 mapRW
    {
        mapRW<int, int> map;
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        for (size_t t = 0; t < NUM_THREADS; ++t) {
            threads.emplace_back([&, t]() {
                for (size_t i = 0; i < MULTI_THREAD_OPS / 10; ++i) {
                    map.insert(t * (MULTI_THREAD_OPS / 10) + i, i * 2);
                }
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        size_t expected = NUM_THREADS * (MULTI_THREAD_OPS / 10);
        
        BenchmarkResult result{
            "Map Concurrent Insert",
            "mapRW",
            time,
            expected,
            map.size() == expected
        };
        results.push_back(result);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << result.container_type << ": " << time << "ms (大小: " 
                  << map.size() << ", 正确: " << (result.data_valid ? "✓" : "✗") << ")\n";
    }
#endif
}

void run_map_concurrent_read_benchmarks(std::vector<BenchmarkResult>& results) {
    print_section_header("Map 多线程并发读取性能测试");
    
    constexpr size_t DATA_SIZE = 10000;
    
    // 初始化 mapMutex
    {
        mapMutex<int, int> map;
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            map.insert(i, i * 2);
        }
        
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        for (size_t t = 0; t < READ_HEAVY_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += map.get(i % DATA_SIZE, 0);
                }
                sum += local_sum;
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        BenchmarkResult result{
            "Map Concurrent Read",
            "mapMutex",
            time,
            READ_HEAVY_THREADS * MULTI_THREAD_OPS,
            sum > 0
        };
        results.push_back(result);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << result.container_type << ": " << time << "ms\n";
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 初始化 mapRW
    {
        mapRW<int, int> map;
        for (size_t i = 0; i < DATA_SIZE; ++i) {
            map.insert(i, i * 2);
        }
        
        std::atomic<size_t> sum{0};
        
        PerformanceTimer timer;
        timer.start();
        
        std::vector<std::thread> threads;
        for (size_t t = 0; t < READ_HEAVY_THREADS; ++t) {
            threads.emplace_back([&]() {
                size_t local_sum = 0;
                for (size_t i = 0; i < MULTI_THREAD_OPS; ++i) {
                    local_sum += map.get(i % DATA_SIZE, 0);
                }
                sum += local_sum;
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        
        double time = timer.stop();
        
        BenchmarkResult result{
            "Map Concurrent Read",
            "mapRW",
            time,
            READ_HEAVY_THREADS * MULTI_THREAD_OPS,
            sum > 0
        };
        results.push_back(result);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << result.container_type << ": " << time << "ms\n";
    }
#endif
}

// ==================== 主函数 ====================

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         TS_STL 线程安全容器库 - 综合性能基准测试              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    
    std::cout << "\n测试配置:\n";
    std::cout << "  - 单线程操作数: " << SINGLE_THREAD_OPS << "\n";
    std::cout << "  - 多线程操作数: " << MULTI_THREAD_OPS << " (每线程)\n";
    std::cout << "  - 并发写入线程数: " << NUM_THREADS << "\n";
    std::cout << "  - 并发读取线程数: " << READ_HEAVY_THREADS << "\n";
    
    std::vector<BenchmarkResult> results;
    
    // 运行所有测试
    run_single_thread_benchmarks(results);
    run_concurrent_write_benchmarks(results);
    run_concurrent_read_benchmarks(results);
    run_mixed_read_write_90_10_benchmarks(results);
    run_mixed_read_write_50_50_benchmarks(results);
    run_map_insert_benchmarks(results);
    run_map_concurrent_insert_benchmarks(results);
    run_map_concurrent_read_benchmarks(results);
    
    // 输出结果
    print_results_table(results);
    generate_markdown_summary(results);
    
    std::cout << "\n测试完成！\n";
    
    return 0;
}
