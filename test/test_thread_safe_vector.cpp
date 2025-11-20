#include "../include/ts_stl.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

using namespace ts_stl;

// ==================== 测试1: 基础操作 ====================
void test_basic_operations() {
    std::cout << "\n=== Test 1: Basic Operations ===" << std::endl;

    vector<int> vec;
    
    // 测试push_back
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    assert(vec.size() == 3);
    std::cout << "✓ push_back and size() work" << std::endl;

    // 测试get
    assert(vec.get(0) == 1);
    assert(vec.get(1) == 2);
    assert(vec.get(2) == 3);
    std::cout << "✓ get() works" << std::endl;

    // 测试set
    vec.set(1, 20);
    assert(vec.get(1) == 20);
    std::cout << "✓ set() works" << std::endl;

    // 测试empty
    assert(!vec.empty());
    std::cout << "✓ empty() works" << std::endl;

    // 测试front/back
    assert(vec.front() == 1);
    assert(vec.back() == 3);
    std::cout << "✓ front() and back() work" << std::endl;
}

// ==================== 测试2: 容量操作 ====================
void test_capacity_operations() {
    std::cout << "\n=== Test 2: Capacity Operations ===" << std::endl;

    vector<std::string> vec;
    
    // 测试reserve
    vec.reserve(10);
    assert(vec.capacity() >= 10);
    std::cout << "✓ reserve() works" << std::endl;

    // 测试resize
    vec.resize(5, "default");
    assert(vec.size() == 5);
    assert(vec.get(0) == "default");
    std::cout << "✓ resize() works" << std::endl;

    // 测试clear
    vec.clear();
    assert(vec.empty());
    assert(vec.size() == 0);
    std::cout << "✓ clear() works" << std::endl;
}

// ==================== 测试3: 拷贝构造和赋值 ====================
void test_copy_operations() {
    std::cout << "\n=== Test 3: Copy Operations ===" << std::endl;

    vector<int> vec1;
    vec1.push_back(1);
    vec1.push_back(2);
    vec1.push_back(3);

    // 拷贝构造
    vector<int> vec2(vec1);
    assert(vec2.size() == 3);
    assert(vec2.get(0) == 1);
    std::cout << "✓ Copy constructor works" << std::endl;

    // 拷贝赋值
    vector<int> vec3;
    vec3 = vec1;
    assert(vec3.size() == 3);
    assert(vec3.get(1) == 2);
    std::cout << "✓ Copy assignment works" << std::endl;

    // 移动构造
    vector<int> vec4(std::move(vec3));
    assert(vec4.size() == 3);
    assert(vec3.size() == 0);  // vec3 被移动后为空
    std::cout << "✓ Move constructor works" << std::endl;
}

// ==================== 测试4: 多线程并发访问 ====================
void test_concurrent_access() {
    std::cout << "\n=== Test 4: Concurrent Access ===" << std::endl;

    vector<int> vec;
    vec.reserve(1000);

    const int num_threads = 4;
    const int operations_per_thread = 250;
    
    std::vector<std::thread> threads;

    // 创建多个线程同时进行push_back操作
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&vec, i]() {
            for (int j = 0; j < 250; ++j) {
                vec.push_back(i * 250 + j);
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    assert(vec.size() == num_threads * operations_per_thread);
    std::cout << "✓ Multiple threads concurrent push_back: " 
              << vec.size() << " elements" << std::endl;
}

// ==================== 测试5: 读写锁策略 ====================
void test_read_write_lock_policy() {
    std::cout << "\n=== Test 5: Read-Write Lock Policy ===" << std::endl;

    vectorRW<int> vec;  // 使用读写锁
    
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }

    std::vector<std::thread> threads;
    
    // 创建多个读线程
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&vec]() {
            for (int j = 0; j < 100; ++j) {
                auto size = vec.size();
                (void)size;  // 避免未使用变量警告
            }
        });
    }

    // 创建一个写线程
    threads.emplace_back([&vec]() {
        for (int i = 10; i < 20; ++i) {
            vec.push_back(i);
        }
    });

    for (auto& t : threads) {
        t.join();
    }

    assert(vec.size() == 20);
    std::cout << "✓ Read-Write lock policy works with concurrent read/write" << std::endl;
}

// ==================== 测试6: emplace_back ====================
void test_emplace_back() {
    std::cout << "\n=== Test 6: Emplace Back ===" << std::endl;

    struct TestObj {
        int x;
        std::string y;
        TestObj(int a, const std::string& b) : x(a), y(b) {}
    };

    vector<TestObj> vec;
    vec.emplace_back(42, "hello");
    vec.emplace_back(99, "world");

    assert(vec.size() == 2);
    assert(vec.get(0).x == 42);
    assert(vec.get(0).y == "hello");
    std::cout << "✓ emplace_back works" << std::endl;
}

// ==================== 测试7: 隐式转换 ====================
void test_implicit_conversion() {
    std::cout << "\n=== Test 7: Implicit Conversion ===" << std::endl;

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    // 隐式转换到const std::vector<int>&
    const std::vector<int>& ref = vec;
    assert(ref.size() == 3);
    assert(ref[0] == 1);
    std::cout << "✓ Implicit conversion to const std::vector& works" << std::endl;

    // 使用copy()获取拷贝
    std::vector<int> copy_vec = vec.to_vector();
    assert(copy_vec.size() == 3);
    std::cout << "✓ to_vector() conversion works" << std::endl;
}

// ==================== 测试8: 迭代和查找 ====================
void test_iteration_and_find() {
    std::cout << "\n=== Test 8: Iteration and Find ===" << std::endl;

    vector<int> vec;
    for (int i = 1; i <= 5; ++i) {
        vec.push_back(i * 10);
    }

    // 测试contains
    assert(vec.contains(30));
    assert(!vec.contains(35));
    std::cout << "✓ contains() works" << std::endl;

    // 测试for_each
    int sum = 0;
    vec.for_each([&sum](int val) { sum += val; });
    assert(sum == 150);  // 10 + 20 + 30 + 40 + 50
    std::cout << "✓ for_each() works, sum = " << sum << std::endl;
}

// ==================== 测试9: 异常处理 ====================
void test_exception_handling() {
    std::cout << "\n=== Test 9: Exception Handling ===" << std::endl;

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);

    // 测试越界访问
    try {
        vec.get(10);
        assert(false);  // 应该抛出异常
    } catch (const std::out_of_range& e) {
        std::cout << "✓ Out of range exception caught: " << e.what() << std::endl;
    }

    // 测试空容器操作
    try {
        vector<int> empty_vec;
        empty_vec.pop_back();
        assert(false);
    } catch (const std::out_of_range& e) {
        std::cout << "✓ Pop from empty vector exception caught" << std::endl;
    }
}

// ==================== 测试10: 互斥锁与读写锁对比 ====================
void test_lock_policies_comparison() {
    std::cout << "\n=== Test 10: Lock Policies Comparison ===" << std::endl;

    vectorMutex<int> mutex_vec;
    vectorRW<int> rw_vec;

    for (int i = 0; i < 100; ++i) {
        mutex_vec.push_back(i);
        rw_vec.push_back(i);
    }

    assert(mutex_vec.size() == 100);
    assert(rw_vec.size() == 100);
    std::cout << "✓ Both Mutex and ReadWrite lock policies work" << std::endl;
    std::cout << "  - Mutex: simpler, suitable for general cases" << std::endl;
    std::cout << "  - ReadWrite: allows concurrent reads, better for read-heavy workloads" 
              << std::endl;
}

// ==================== 主测试函数 ====================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Thread-Safe STL Vector Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_basic_operations();
        test_capacity_operations();
        test_copy_operations();
        test_concurrent_access();
        test_read_write_lock_policy();
        test_emplace_back();
        test_implicit_conversion();
        test_iteration_and_find();
        test_exception_handling();
        test_lock_policies_comparison();

        std::cout << "\n========================================" << std::endl;
        std::cout << "✅ All tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
