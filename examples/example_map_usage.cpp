#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "ts_stl.hpp"

using namespace ts_stl;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Thread-Safe STL Map Example" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    // ==================== 基础使用 ====================
    std::cout << "=== 基础使用 ===" << std::endl;
    
    mapMutex<std::string, int> scores;
    
    // 插入数据
    scores.insert("Alice", 90);
    scores.insert("Bob", 85);
    scores.insert("Charlie", 88);
    
    std::cout << "Inserted 3 scores" << std::endl;
    std::cout << "Map size: " << scores.size() << std::endl;
    
    // 访问数据
    std::cout << "Alice's score: " << scores.get("Alice") << std::endl;
    std::cout << "Bob's score: " << scores.get("Bob") << std::endl;
    
    // 检查是否包含
    std::cout << "Contains 'Charlie': " << (scores.contains("Charlie") ? "true" : "false") << std::endl;
    std::cout << "Contains 'David': " << (scores.contains("David") ? "true" : "false") << std::endl;
    
    std::cout << std::endl;

    // ==================== 修改操作 ====================
    std::cout << "=== 修改操作 ===" << std::endl;
    
    // 更新值
    scores.set("Alice", 95);
    std::cout << "Updated Alice's score to 95: " << scores.get("Alice") << std::endl;
    
    // 移除元素
    size_t removed = scores.erase("Bob");
    std::cout << "Removed " << removed << " element(s)" << std::endl;
    std::cout << "Map size after removal: " << scores.size() << std::endl;
    
    std::cout << std::endl;

    // ==================== 遍历操作 ====================
    std::cout << "=== 遍历操作 ===" << std::endl;
    
    // 添加更多数据
    scores.insert("Diana", 92);
    scores.insert("Eve", 87);
    
    // for_each 遍历
    std::cout << "All scores:" << std::endl;
    scores.for_each([](const auto& key, const auto& value) {
        std::cout << "  " << key << ": " << value << std::endl;
    });
    
    std::cout << std::endl;

    // ==================== 并发访问 ====================
    std::cout << "=== 并发访问 ===" << std::endl;
    
    mapMutex<int, std::string> cache;
    
    // 创建多个线程向map中插入数据
    std::vector<std::thread> threads;
    for (int t = 0; t < 3; ++t) {
        threads.emplace_back([&cache, t]() {
            for (int i = 0; i < 5; ++i) {
                int key = t * 5 + i;
                std::string value = "value_" + std::to_string(key);
                cache.insert(key, value);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Concurrent insertion completed" << std::endl;
    std::cout << "Cache size: " << cache.size() << std::endl;
    
    std::cout << std::endl;

    // ==================== 读写锁示例 ====================
#if TS_STL_SUPPORT_RW_LOCK
    std::cout << "=== 读写锁示例 ===" << std::endl;
    
    mapRW<std::string, double> data;
    
    // 插入初始数据
    data.insert("price", 99.99);
    data.insert("quantity", 100.0);
    
    // 演示多个并发读
    std::vector<std::thread> readers;
    for (int i = 0; i < 2; ++i) {
        readers.emplace_back([&data, i]() {
            for (int j = 0; j < 3; ++j) {
                std::cout << "Reader " << i << " reading..." << std::endl;
                data.with_read_lock([i](const auto& map_ref) {
                    (void)map_ref;  // 避免未使用警告
                    std::cout << "  Reader " << i << " sees " << map_ref.size() << " items" << std::endl;
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    // 等待读线程完成
    for (auto& thread : readers) {
        thread.join();
    }
    
    std::cout << "Read operations completed" << std::endl;
    
    std::cout << std::endl;
#endif

    // ==================== 手动锁控制 ====================
    std::cout << "=== 手动锁控制 ===" << std::endl;
    
    mapMutex<int, std::string> manual_lock_map;
    
    // 批量操作，只加一次锁
    {
        auto guard = manual_lock_map.acquire_write_guard();
        for (int i = 0; i < 5; ++i) {
            manual_lock_map.unsafe_insert(i, "item_" + std::to_string(i));
        }
    }
    
    std::cout << "Batch insertion completed, size: " << manual_lock_map.size() << std::endl;
    
    std::cout << std::endl;

    // ==================== 移动语义 ====================
    std::cout << "=== 移动语义 ===" << std::endl;
    
    mapMutex<std::string, std::string> dict1;
    dict1.insert("greeting", "Hello");
    dict1.insert("farewell", "Goodbye");
    
    // 移动构造
    mapMutex<std::string, std::string> dict2 = std::move(dict1);
    
    std::cout << "dict1 size after move: " << dict1.size() << std::endl;
    std::cout << "dict2 size after move: " << dict2.size() << std::endl;
    std::cout << "dict2['greeting']: " << dict2.get("greeting") << std::endl;
    
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "All examples completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
