#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include "ts_stl.hpp"

using namespace ts_stl;

/**
 * @brief Thread-safe Unordered Map 单元测试
 */

// ==================== 基础功能测试 ====================
void test_basic_operations() {
    std::cout << "Testing basic operations..." << std::endl;
    
    unordered_mapMutex<std::string, int> map;
    
    // 测试empty
    assert(map.empty());
    
    // 测试insert
    map.insert("key1", 10);
    map.insert("key2", 20);
    assert(map.size() == 2);
    
    // 测试get
    assert(map.get("key1") == 10);
    assert(map.get("key2") == 20);
    assert(map.get("key3", -1) == -1);
    
    // 测试set
    map.set("key1", 15);
    assert(map.get("key1") == 15);
    
    // 测试contains
    assert(map.contains("key1"));
    assert(!map.contains("key3"));
    
    // 测试erase
    map.erase("key2");
    assert(map.size() == 1);
    assert(!map.contains("key2"));
    
    // 测试clear
    map.clear();
    assert(map.empty());
    
    std::cout << "✓ Basic operations passed" << std::endl;
}

// ==================== 容量管理测试 ====================
void test_capacity_management() {
    std::cout << "Testing capacity management..." << std::endl;
    
    unordered_mapMutex<int, int> map;
    
    // 初始大小
    assert(map.empty());
    assert(map.size() == 0);
    
    // 插入元素
    for (int i = 0; i < 100; ++i) {
        map.insert(i, i * 2);
    }
    assert(map.size() == 100);
    
    // 测试reserve
    map.reserve(500);
    assert(map.size() == 100);  // Size不变
    
    // 测试bucket_count
    assert(map.bucket_count() > 0);
    
    // 测试load_factor
    float load = map.load_factor();
    assert(load > 0.0f);
    
    std::cout << "✓ Capacity management passed" << std::endl;
}

// ==================== 并发操作测试 ====================
void test_concurrent_operations() {
    std::cout << "Testing concurrent operations..." << std::endl;
    
    unordered_mapMutex<int, int> shared_map;
    
    const int NUM_THREADS = 10;
    const int OPERATIONS_PER_THREAD = 100;
    std::vector<std::thread> threads;
    
    // 并发写入
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&shared_map, t]() {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                int key = t * OPERATIONS_PER_THREAD + i;
                shared_map.insert(key, key * 2);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有数据都正确写入
    assert(shared_map.size() == NUM_THREADS * OPERATIONS_PER_THREAD);
    
    for (int i = 0; i < NUM_THREADS * OPERATIONS_PER_THREAD; ++i) {
        assert(shared_map.get(i) == i * 2);
    }
    
    std::cout << "✓ Concurrent operations passed" << std::endl;
}

// ==================== 并发读写混合测试 ====================
void test_concurrent_read_write() {
    std::cout << "Testing concurrent read/write..." << std::endl;
    
    unordered_mapMutex<std::string, int> shared_map;
    
    // 初始化数据
    for (int i = 0; i < 50; ++i) {
        shared_map.insert("key_" + std::to_string(i), i);
    }
    
    std::vector<std::thread> threads;
    
    // 5个写线程
    for (int t = 0; t < 5; ++t) {
        threads.emplace_back([&shared_map, t]() {
            for (int i = 0; i < 50; ++i) {
                std::string key = "key_" + std::to_string(i);
                shared_map.set(key, i + t * 50);
            }
        });
    }
    
    // 5个读线程
    for (int t = 0; t < 5; ++t) {
        threads.emplace_back([&shared_map]() {
            for (int i = 0; i < 100; ++i) {
                std::string key = "key_" + std::to_string(i % 50);
                int value = shared_map.get(key, -1);
                (void)value;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    assert(shared_map.size() == 50);
    
    std::cout << "✓ Concurrent read/write passed" << std::endl;
}

// ==================== 迭代和查询测试 ====================
void test_iteration_and_query() {
    std::cout << "Testing iteration and query..." << std::endl;
    
    unordered_mapMutex<int, int> map;
    
    // 插入测试数据
    for (int i = 0; i < 20; ++i) {
        map.insert(i, i * 3);
    }
    
    // 测试for_each
    int sum = 0;
    map.for_each([&sum](const auto& key, const auto& value) {
        sum += value;
    });
    assert(sum == (0 + 19) * 20 / 2 * 3);  // 0+3+6+...+57
    
    // 测试count_if
    size_t count = map.count_if([](const auto& key, const auto& value) {
        return value > 30;
    });
    assert(count > 0);
    
    std::cout << "✓ Iteration and query passed" << std::endl;
}

// ==================== LockFree版本测试 ====================
void test_lockfree_version() {
    std::cout << "Testing LockFree version..." << std::endl;
    
    unordered_mapLockFree<int, std::string> map;
    
    // 单线程操作（LockFree需要外部同步）
    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");
    
    assert(map.size() == 3);
    assert(map.get(1) == "one");
    assert(map.get(2) == "two");
    assert(map.get(3) == "three");
    
    map.erase(2);
    assert(map.size() == 2);
    assert(!map.contains(2));
    
    std::cout << "✓ LockFree version passed" << std::endl;
}

// ==================== 异常安全测试 ====================
void test_exception_safety() {
    std::cout << "Testing exception safety..." << std::endl;
    
    unordered_mapMutex<int, int> map;
    
    // 即使发生异常，map状态也应该保持有效
    try {
        for (int i = 0; i < 100; ++i) {
            map.insert(i, i * 2);
        }
        // 虽然at会抛异常，但map状态保持有效
        int value = map.at(9999);
        (void)value;
    } catch (const std::out_of_range&) {
        // 预期的异常
    }
    
    // map应该仍然有效
    assert(map.size() == 100);
    assert(map.get(0) == 0);
    assert(map.get(99) == 198);
    
    std::cout << "✓ Exception safety passed" << std::endl;
}

// ==================== 手动锁控制测试 ====================
void test_manual_lock_control() {
    std::cout << "Testing manual lock control..." << std::endl;
    
    unordered_mapMutex<std::string, int> map;
    
    // 测试with_write_lock
    map.with_write_lock([](auto& m) {
        for (int i = 0; i < 50; ++i) {
            m.unsafe_insert("key_" + std::to_string(i), i);
        }
    });
    
    assert(map.size() == 50);
    
    // 测试acquire_write_guard
    {
        auto guard = map.acquire_write_guard();
        map.unsafe_insert("extra", 999);
    }
    
    assert(map.size() == 51);
    
    std::cout << "✓ Manual lock control passed" << std::endl;
}

int main() {
    std::cout << "Thread-Safe Unordered Map Unit Tests" << std::endl;
    std::cout << "====================================" << std::endl;
    
    try {
        test_basic_operations();
        test_capacity_management();
        test_concurrent_operations();
        test_concurrent_read_write();
        test_iteration_and_query();
        test_lockfree_version();
        test_exception_safety();
        test_manual_lock_control();
        
        std::cout << "\n✓ All tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
