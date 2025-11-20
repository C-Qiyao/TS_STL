#include "../include/ts_stl.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

using namespace ts_stl;

// ==================== 测试1: 线程不安全接口 ====================
void test_unsafe_interfaces() {
    std::cout << "\n=== Test: Unsafe Interfaces ===" << std::endl;

    vector<int> vec;
    
    // 使用线程不安全接口 - 在单线程环境下
    vec.with_write_lock([](auto& v) {
        v.unsafe_push_back(1);
        v.unsafe_push_back(2);
        v.unsafe_push_back(3);
    });

    // 验证结果
    assert(vec.unsafe_size() == 3);
    std::cout << "✓ unsafe_push_back via with_write_lock works" << std::endl;

    // 直接获取unsafe引用
    auto& ref = vec.unsafe_ref();
    assert(ref[0] == 1);
    assert(ref[1] == 2);
    std::cout << "✓ unsafe_ref() works" << std::endl;

    // 线程不安全访问
    assert(vec.unsafe_at(0) == 1);
    vec.unsafe_at(1) = 20;
    assert(vec.unsafe_at(1) == 20);
    std::cout << "✓ unsafe_at() works" << std::endl;

    // 线程不安全的empty和size
    assert(!vec.unsafe_empty());
    assert(vec.unsafe_size() == 3);
    std::cout << "✓ unsafe_empty() and unsafe_size() work" << std::endl;

    // 线程不安全的clear
    vec.with_write_lock([](auto& v) {
        v.unsafe_clear();
    });
    assert(vec.unsafe_empty());
    std::cout << "✓ unsafe_clear() works" << std::endl;
}

// ==================== 测试2: 手动锁控制 ====================
void test_manual_lock_control() {
    std::cout << "\n=== Test: Manual Lock Control ===" << std::endl;

    vector<int> vec;

    // 方式1: 获取写锁guard进行多个操作
    {
        auto guard = vec.acquire_write_guard();
        vec.unsafe_push_back(10);
        vec.unsafe_push_back(20);
        vec.unsafe_push_back(30);
    }  // guard 自动释放

    assert(vec.size() == 3);
    std::cout << "✓ acquire_write_guard() works" << std::endl;

    // 方式2: with_write_lock 进行批量操作
    vec.with_write_lock([](auto& v) {
        v.unsafe_resize(5);
        // 所有操作都在同一个锁内完成
    });

    assert(vec.unsafe_size() == 5);
    std::cout << "✓ with_write_lock() for batch operations works" << std::endl;
}

// ==================== 测试3: 读锁接口（C++17+） ====================
#if TS_STL_SUPPORT_RW_LOCK
void test_read_lock_interfaces() {
    std::cout << "\n=== Test: Read Lock Interfaces (C++17+) ===" << std::endl;

    vectorRW<int> vec;

    // 添加数据
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    // 方式1: 获取读锁guard
    {
        auto guard = vec.acquire_read_guard();
        assert(vec.unsafe_at(0) == 1);
        assert(vec.unsafe_size() == 3);
    }

    std::cout << "✓ acquire_read_guard() works" << std::endl;

    // 方式2: with_read_lock 进行只读操作
    vec.with_read_lock([](const auto& v) {
        assert(v.unsafe_empty() == false);
        assert(v.unsafe_size() == 3);
    });

    std::cout << "✓ with_read_lock() works" << std::endl;

    // 多个读线程可以并发访问
    std::vector<std::thread> threads;
    int read_count = 0;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&vec, &read_count]() {
            for (int j = 0; j < 10; ++j) {
                auto guard = vec.acquire_read_guard();
                auto size = vec.unsafe_size();
                (void)size;  // 使用size变量
                read_count++;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "✓ Multiple threads can concurrently read with RW lock" << std::endl;
}
#endif

// ==================== 测试4: 复杂场景 ====================
void test_complex_scenarios() {
    std::cout << "\n=== Test: Complex Scenarios ===" << std::endl;

    vector<std::pair<int, std::string>> vec;

    // 场景1: 批量插入（原子操作）
    vec.with_write_lock([](auto& v) {
        v.unsafe_push_back({1, "one"});
        v.unsafe_push_back({2, "two"});
        v.unsafe_push_back({3, "three"});
    });

    assert(vec.unsafe_size() == 3);
    std::cout << "✓ Batch insertion works" << std::endl;

    // 场景2: 批量修改
    vec.with_write_lock([](auto& v) {
        for (auto& item : v.unsafe_ref()) {
            item.second += "_modified";
        }
    });

    // 验证
    vec.for_each([](const auto& item) {
        assert(item.second.find("_modified") != std::string::npos);
    });
    std::cout << "✓ Batch modification works" << std::endl;

    // 场景3: 条件清理
    vec.with_write_lock([](auto& v) {
        auto& ref = v.unsafe_ref();
        ref.erase(
            std::remove_if(ref.begin(), ref.end(),
                [](const auto& item) { return item.first == 2; }),
            ref.end()
        );
    });

    assert(vec.unsafe_size() == 2);
    std::cout << "✓ Conditional erase works" << std::endl;
}

// ==================== 测试5: 性能对比 ====================
void test_performance_comparison() {
    std::cout << "\n=== Test: Performance Comparison ===" << std::endl;

    vector<int> vec;
    vec.reserve(1000);

    // 方法1: 单个元素操作（每个操作一个锁）
    {
        for (int i = 0; i < 100; ++i) {
            vec.push_back(i);
        }
    }
    std::cout << "✓ Individual push_back x100 works" << std::endl;

    // 方法2: 批量操作（一个锁保护多个操作）
    {
        vec.with_write_lock([](auto& v) {
            for (int i = 0; i < 100; ++i) {
                v.unsafe_push_back(i + 100);
            }
        });
    }
    std::cout << "✓ Batch push_back x100 (single lock) works" << std::endl;

    assert(vec.unsafe_size() == 200);
}

// ==================== 主测试函数 ====================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Advanced Thread-Safe Vector Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "C++ Version Features:" << std::endl;
    std::cout << "  - ReadWrite Lock Support: " 
#if TS_STL_SUPPORT_RW_LOCK
              << "YES (C++17+)"
#else
              << "NO (C++<17)"
#endif
              << std::endl;

    try {
        test_unsafe_interfaces();
        test_manual_lock_control();

#if TS_STL_SUPPORT_RW_LOCK
        test_read_lock_interfaces();
#endif

        test_complex_scenarios();
        test_performance_comparison();

        std::cout << "\n========================================" << std::endl;
        std::cout << "✅ All advanced tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
