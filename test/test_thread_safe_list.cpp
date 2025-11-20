#include "../include/ts_stl.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

using namespace ts_stl;

// ==================== 测试1: List基本操作 ====================
void test_list_basic_operations() {
    std::cout << "\n=== Test 1: List Basic Operations ===" << std::endl;

    list<int> list;
    
    // push_back
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    
    assert(list.size() == 3);
    std::cout << "✓ push_back and size() work" << std::endl;

    // front/back
    assert(list.front() == 1);
    assert(list.back() == 3);
    std::cout << "✓ front() and back() work" << std::endl;

    // set_front/set_back
    list.set_front(10);
    list.set_back(30);
    assert(list.front() == 10);
    assert(list.back() == 30);
    std::cout << "✓ set_front() and set_back() work" << std::endl;

    // empty
    assert(!list.empty());
    std::cout << "✓ empty() works" << std::endl;
}

// ==================== 测试2: List首部操作 ====================
void test_list_front_operations() {
    std::cout << "\n=== Test 2: List Front Operations ===" << std::endl;

    list<std::string> list;
    
    // push_front
    list.push_front("first");
    list.push_front("second");
    list.push_front("third");
    
    assert(list.size() == 3);
    assert(list.front() == "third");
    std::cout << "✓ push_front() works" << std::endl;

    // pop_front
    list.pop_front();
    assert(list.front() == "second");
    assert(list.size() == 2);
    std::cout << "✓ pop_front() works" << std::endl;

    // emplace_front
    list.emplace_front("brand_new");
    assert(list.front() == "brand_new");
    std::cout << "✓ emplace_front() works" << std::endl;
}

// ==================== 测试3: List移除操作 ====================
void test_list_remove_operations() {
    std::cout << "\n=== Test 3: List Remove Operations ===" << std::endl;

    list<int> list;
    for (int i = 1; i <= 10; ++i) {
        list.push_back(i);
    }

    // remove - 移除所有3的倍数
    size_t removed = list.remove_if([](int x) { return x % 3 == 0; });
    assert(removed == 3);  // 3, 6, 9 被移除
    assert(list.size() == 7);
    std::cout << "✓ remove_if() works, removed " << removed << " elements" << std::endl;

    // clear
    list.clear();
    assert(list.empty());
    std::cout << "✓ clear() works" << std::endl;
}

// ==================== 测试4: List排序和反转 ====================
void test_list_sort_and_reverse() {
    std::cout << "\n=== Test 4: List Sort and Reverse ===" << std::endl;

    list<int> list;
    for (int i : {5, 2, 8, 1, 9, 3}) {
        list.push_back(i);
    }

    // sort
    list.sort();
    auto copy1 = list.copy();
    assert(copy1.front() == 1);
    assert(copy1.back() == 9);
    std::cout << "✓ sort() works" << std::endl;

    // reverse
    list.reverse();
    auto copy2 = list.copy();
    assert(copy2.front() == 9);
    assert(copy2.back() == 1);
    std::cout << "✓ reverse() works" << std::endl;
}

// ==================== 测试5: List查询操作 ====================
void test_list_query_operations() {
    std::cout << "\n=== Test 5: List Query Operations ===" << std::endl;

    list<int> list;
    for (int i = 1; i <= 5; ++i) {
        list.push_back(i * 10);
    }

    // contains
    assert(list.contains(30));
    assert(!list.contains(35));
    std::cout << "✓ contains() works" << std::endl;

    // count
    list.push_back(30);
    auto cnt = list.count(30);
    assert(cnt == 2);
    std::cout << "✓ count() works" << std::endl;

    // for_each
    int sum = 0;
    list.for_each([&sum](int val) { sum += val; });
    std::cout << "✓ for_each() works, sum = " << sum << std::endl;
}

// ==================== 测试6: List拷贝和移动 ====================
void test_list_copy_operations() {
    std::cout << "\n=== Test 6: List Copy Operations ===" << std::endl;

    list<int> list1;
    list1.push_back(1);
    list1.push_back(2);
    list1.push_back(3);

    // 拷贝构造
    list<int> list2(list1);
    assert(list2.size() == 3);
    assert(list2.front() == 1);
    std::cout << "✓ Copy constructor works" << std::endl;

    // 拷贝赋值
    list<int> list3;
    list3 = list1;
    assert(list3.size() == 3);
    std::cout << "✓ Copy assignment works" << std::endl;

    // 移动构造
    list<int> list4(std::move(list3));
    assert(list4.size() == 3);
    assert(list3.size() == 0);
    std::cout << "✓ Move constructor works" << std::endl;
}

// ==================== 测试7: 多线程并发 ====================
void test_list_concurrent_access() {
    std::cout << "\n=== Test 7: List Concurrent Access ===" << std::endl;

    list<int> list;
    
    std::vector<std::thread> threads;
    
    // 4 个线程，每个添加 50 个元素
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&list, i]() {
            for (int j = 0; j < 50; ++j) {
                list.push_back(i * 50 + j);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    assert(list.size() == 200);
    std::cout << "✓ Concurrent push_back: " << list.size() << " elements" << std::endl;
}

// ==================== 测试8: 手动锁控制 ====================
void test_list_manual_lock() {
    std::cout << "\n=== Test 8: List Manual Lock Control ===" << std::endl;

    list<int> list;

    // 批量操作
    list.with_write_lock([](auto& l) {
        l.unsafe_push_back(1);
        l.unsafe_push_back(2);
        l.unsafe_push_back(3);
    });

    assert(list.unsafe_size() == 3);
    std::cout << "✓ with_write_lock() works" << std::endl;

    // 获取guard
    {
        auto guard = list.acquire_write_guard();
        list.unsafe_push_back(4);
    }

    assert(list.unsafe_size() == 4);
    std::cout << "✓ acquire_write_guard() works" << std::endl;
}

// ==================== 测试9: 复杂数据类型 ====================
void test_list_complex_types() {
    std::cout << "\n=== Test 9: List Complex Types ===" << std::endl;

    struct Item {
        int id;
        std::string name;
        Item(int i, const std::string& n) : id(i), name(n) {}
    };

    list<Item> list;
    
    list.emplace_back(1, "Alice");
    list.emplace_back(2, "Bob");
    list.emplace_back(3, "Charlie");

    assert(list.size() == 3);
    std::cout << "✓ emplace_back with complex types works" << std::endl;

    list.for_each([](const Item& item) {
        // 验证数据
        assert(!item.name.empty());
    });
    std::cout << "✓ for_each with complex types works" << std::endl;
}

// ==================== 测试10: List特定操作 ====================
void test_list_specific_operations() {
    std::cout << "\n=== Test 10: List Specific Operations ===" << std::endl;

    list<int> list;
    for (int i = 1; i <= 5; ++i) {
        list.push_back(i);
    }

    // 移除值为3的所有元素
    size_t removed = list.remove(3);
    assert(removed == 1);
    assert(list.size() == 4);
    std::cout << "✓ remove() works" << std::endl;

    // 改变大小
    list.resize(6, 99);
    assert(list.size() == 6);
    assert(list.back() == 99);
    std::cout << "✓ resize() works" << std::endl;

    // 隐式转换
    const std::list<int>& std_list = list;
    assert(std_list.size() == 6);
    std::cout << "✓ Implicit conversion works" << std::endl;
}

// ==================== 主测试函数 ====================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Thread-Safe List Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_list_basic_operations();
        test_list_front_operations();
        test_list_remove_operations();
        test_list_sort_and_reverse();
        test_list_query_operations();
        test_list_copy_operations();
        test_list_concurrent_access();
        test_list_manual_lock();
        test_list_complex_types();
        test_list_specific_operations();

        std::cout << "\n========================================" << std::endl;
        std::cout << "✅ All list tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
