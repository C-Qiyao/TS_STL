#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "ts_stl.hpp"

using namespace ts_stl;

/**
 * @brief Unordered Map 使用示例
 */

// ==================== 基础使用 ====================
void example_basic_usage() {
    std::cout << "\n========== 基础使用 ==========" << std::endl;
    
    // 创建线程安全的unordered_map
    unordered_mapMutex<std::string, int> cache;
    
    // 插入数据
    cache.insert("Alice", 90);
    cache.insert("Bob", 85);
    cache.insert("Charlie", 92);
    
    // 获取数据
    std::cout << "Alice's score: " << cache.get("Alice", -1) << std::endl;
    std::cout << "David's score: " << cache.get("David", -1) << std::endl;
    
    // 检查是否存在
    if (cache.contains("Bob")) {
        std::cout << "Bob found in cache!" << std::endl;
    }
    
    // 更新数据
    cache.set("Alice", 95);
    std::cout << "Updated Alice's score: " << cache.get("Alice") << std::endl;
    
    // 删除数据
    cache.erase("Charlie");
    std::cout << "Cache size after deletion: " << cache.size() << std::endl;
}

// ==================== 多线程读写 ====================
void example_concurrent_read_write() {
    std::cout << "\n========== 多线程读写 ==========" << std::endl;
    
    unordered_mapMutex<std::string, int> shared_cache;
    
    // 初始化数据
    for (int i = 0; i < 10; ++i) {
        shared_cache.insert("key_" + std::to_string(i), i * 10);
    }
    
    std::vector<std::thread> threads;
    
    // 创建4个写线程
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&shared_cache, t]() {
            for (int i = 0; i < 50; ++i) {
                std::string key = "write_key_" + std::to_string(t * 50 + i);
                shared_cache.insert(key, t * 100 + i);
            }
        });
    }
    
    // 创建4个读线程
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&shared_cache, t]() {
            for (int i = 0; i < 100; ++i) {
                std::string key = "key_" + std::to_string(i % 10);
                int value = shared_cache.get(key, -1);
                (void)value;  // 避免编译器警告
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Final cache size: " << shared_cache.size() << std::endl;
}

// ==================== 读写锁版本（如果支持） ====================
void example_read_write_lock() {
#if TS_STL_SUPPORT_RW_LOCK
    std::cout << "\n========== 读写锁版本 ==========" << std::endl;
    
    // 使用读写锁的unordered_map，适合读多写少的场景
    unordered_mapRW<std::string, int> config;
    
    // 初始化配置
    config.insert("timeout", 3000);
    config.insert("retries", 5);
    config.insert("buffer_size", 4096);
    
    std::vector<std::thread> threads;
    
    // 创建8个读线程
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&config, t]() {
            for (int i = 0; i < 1000; ++i) {
                int value = config.get("timeout", -1);
                (void)value;
            }
        });
    }
    
    // 创建1个写线程
    threads.emplace_back([&config]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        config.set("timeout", 5000);
        config.set("retries", 10);
    });
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Config timeout: " << config.get("timeout") << std::endl;
    std::cout << "Config retries: " << config.get("retries") << std::endl;
#else
    std::cout << "\n读写锁版本需要C++17支持" << std::endl;
#endif
}

// ==================== 遍历操作 ====================
void example_iteration() {
    std::cout << "\n========== 遍历操作 ==========" << std::endl;
    
    unordered_mapMutex<std::string, int> scores;
    
    // 插入数据
    scores.insert("Alice", 90);
    scores.insert("Bob", 85);
    scores.insert("Charlie", 92);
    scores.insert("David", 88);
    
    // 遍历所有元素
    std::cout << "All scores:" << std::endl;
    scores.for_each([](const auto& key, const auto& value) {
        std::cout << "  " << key << ": " << value << std::endl;
    });
    
    // 条件统计
    int high_scores = scores.count_if([](const auto& key, const auto& value) {
        return value >= 90;
    });
    std::cout << "Scores >= 90: " << high_scores << std::endl;
}

// ==================== 性能对比 ====================
void example_performance() {
    std::cout << "\n========== 性能对比 ==========" << std::endl;
    
    const int NUM_OPERATIONS = 100000;
    
    // 使用互斥锁
    {
        auto start = std::chrono::high_resolution_clock::now();
        unordered_mapMutex<int, int> map;
        
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            map.insert(i, i * 2);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Mutex version insert " << NUM_OPERATIONS << " elements: " << duration.count() << "ms" << std::endl;
    }
    
    // 使用LockFree
    {
        auto start = std::chrono::high_resolution_clock::now();
        unordered_mapLockFree<int, int> map;
        
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            map.insert(i, i * 2);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "LockFree version insert " << NUM_OPERATIONS << " elements: " << duration.count() << "ms" << std::endl;
    }
}

// ==================== 容量管理 ====================
void example_capacity_management() {
    std::cout << "\n========== 容量管理 ==========" << std::endl;
    
    unordered_mapMutex<std::string, int> map;
    
    // 获取初始容量
    std::cout << "Initial bucket count: " << map.bucket_count() << std::endl;
    
    // 预留空间
    map.reserve(1000);
    std::cout << "After reserve(1000), bucket count: " << map.bucket_count() << std::endl;
    
    // 插入数据
    for (int i = 0; i < 100; ++i) {
        map.insert("key_" + std::to_string(i), i);
    }
    
    std::cout << "After inserting 100 elements:" << std::endl;
    std::cout << "  Size: " << map.size() << std::endl;
    std::cout << "  Bucket count: " << map.bucket_count() << std::endl;
    std::cout << "  Load factor: " << map.load_factor() << std::endl;
}

// ==================== 手动锁控制 ====================
void example_manual_lock_control() {
    std::cout << "\n========== 手动锁控制 ==========" << std::endl;
    
    unordered_mapMutex<std::string, int> map;
    
    // 方式1：使用 with_write_lock 进行批量操作
    map.with_write_lock([](auto& m) {
        for (int i = 0; i < 100; ++i) {
            m.unsafe_insert("batch_" + std::to_string(i), i);
        }
    });
    
    std::cout << "After batch insert: " << map.size() << " elements" << std::endl;
    
    // 方式2：使用 acquire_write_guard
    {
        auto guard = map.acquire_write_guard();
        map.unsafe_insert("manual_1", 100);
        map.unsafe_insert("manual_2", 200);
    }
    
    std::cout << "After manual insert: " << map.size() << " elements" << std::endl;
}

int main() {
    std::cout << "Thread-Safe Unordered Map Examples" << std::endl;
    std::cout << "====================================" << std::endl;
    
    try {
        example_basic_usage();
        example_concurrent_read_write();
        example_read_write_lock();
        example_iteration();
        example_performance();
        example_capacity_management();
        example_manual_lock_control();
        
        std::cout << "\n✓ All examples completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
