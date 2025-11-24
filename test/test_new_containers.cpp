#include <iostream>
#include <cassert>
#include <thread>
#include "ts_stl.hpp"

using namespace ts_stl;

// ==================== Set 测试 ====================
void test_set_basic() {
    std::cout << "Testing Set..." << std::endl;
    
    setMutex<int> s;
    assert(s.empty());
    
    s.insert(1);
    s.insert(2);
    s.insert(3);
    assert(s.size() == 3);
    assert(s.contains(1));
    
    s.erase(2);
    assert(s.size() == 2);
    assert(!s.contains(2));
    
    std::cout << "✓ Set tests passed" << std::endl;
}

// ==================== Unordered Set 测试 ====================
void test_unordered_set_basic() {
    std::cout << "Testing Unordered Set..." << std::endl;
    
    unordered_setMutex<std::string> us;
    assert(us.empty());
    
    us.insert("apple");
    us.insert("banana");
    us.insert("cherry");
    assert(us.size() == 3);
    assert(us.contains("apple"));
    
    us.erase("banana");
    assert(us.size() == 2);
    assert(!us.contains("banana"));
    
    std::cout << "✓ Unordered Set tests passed" << std::endl;
}

// ==================== Deque 测试 ====================
void test_deque_basic() {
    std::cout << "Testing Deque..." << std::endl;
    
    dequeMutex<int> dq;
    assert(dq.empty());
    
    dq.push_back(1);
    dq.push_back(2);
    dq.push_front(0);
    assert(dq.size() == 3);
    
    assert(dq.front() == 0);
    assert(dq.back() == 2);
    
    dq.pop_front();
    assert(dq.front() == 1);
    
    dq.pop_back();
    assert(dq.back() == 1);
    
    std::cout << "✓ Deque tests passed" << std::endl;
}

// ==================== 并发测试 ====================
void test_concurrent_set() {
    std::cout << "Testing concurrent Set operations..." << std::endl;
    
    setMutex<int> shared_set;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&shared_set, t]() {
            for (int i = 0; i < 50; ++i) {
                shared_set.insert(t * 50 + i);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    assert(shared_set.size() == 200);
    std::cout << "✓ Concurrent Set tests passed" << std::endl;
}

void test_concurrent_deque() {
    std::cout << "Testing concurrent Deque operations..." << std::endl;
    
    dequeMutex<int> shared_deque;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&shared_deque, t]() {
            for (int i = 0; i < 50; ++i) {
                if (t % 2 == 0) {
                    shared_deque.push_back(t * 50 + i);
                } else {
                    shared_deque.push_front(t * 50 + i);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    assert(shared_deque.size() == 200);
    std::cout << "✓ Concurrent Deque tests passed" << std::endl;
}

int main() {
    std::cout << "Testing new containers: Set, Unordered Set, Deque" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    try {
        test_set_basic();
        test_unordered_set_basic();
        test_deque_basic();
        test_concurrent_set();
        test_concurrent_deque();
        
        std::cout << "\n✓ All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
