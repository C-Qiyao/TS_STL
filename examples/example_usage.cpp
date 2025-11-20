#include "../include/ts_stl.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace ts_stl;

// ==================== 示例1: 基本使用 ====================
void example_basic_usage() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 1: Basic Usage" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    vector<int> vec;
    
    // 添加元素
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    
    std::cout << "Size: " << vec.size() << std::endl;
    std::cout << "Element at 0: " << vec.get(0) << std::endl;
    std::cout << "Element at 1: " << vec.get(1) << std::endl;
    
    // 修改元素
    vec.set(1, 25);
    std::cout << "After set(1, 25): " << vec.get(1) << std::endl;
}

// ==================== 示例2: 使用不同的锁策略 ====================
void example_lock_strategies() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 2: Lock Strategies" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    // 方法1: 使用互斥锁（默认）
    vectorMutex<std::string> mutex_vec;
    mutex_vec.push_back("hello");
    mutex_vec.push_back("world");
    std::cout << "Using Mutex: [" << mutex_vec.get(0) << ", " 
              << mutex_vec.get(1) << "]" << std::endl;

    // 方法2: 使用读写锁（适合读操作多于写操作的场景）
    vectorRW<std::string> rw_vec;
    rw_vec.push_back("concurrent");
    rw_vec.push_back("read");
    std::cout << "Using ReadWrite Lock: [" << rw_vec.get(0) << ", " 
              << rw_vec.get(1) << "]" << std::endl;
}

// ==================== 示例3: 多线程并发操作 ====================
void example_concurrent_operations() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 3: Concurrent Operations" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    vector<int> counter;
    counter.push_back(0);
    
    std::vector<std::thread> threads;
    
    // 创建多个线程同时进行操作
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&counter, i]() {
            for (int j = 0; j < 20; ++j) {
                counter.push_back(i * 20 + j);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "After 5 threads adding 20 elements each: " 
              << counter.size() << " total elements" << std::endl;
}

// ==================== 示例4: 集合操作 ====================
void example_collection_operations() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 4: Collection Operations" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    vector<int> vec;
    for (int i = 1; i <= 5; ++i) {
        vec.push_back(i * 10);
    }

    // 遍历操作
    std::cout << "Elements: ";
    vec.for_each([](int val) { std::cout << val << " "; });
    std::cout << std::endl;

    // 查找操作
    std::cout << "Contains 30? " << (vec.contains(30) ? "Yes" : "No") << std::endl;
    std::cout << "Contains 35? " << (vec.contains(35) ? "Yes" : "No") << std::endl;

    // 容量操作
    std::cout << "Current size: " << vec.size() << std::endl;
    std::cout << "Current capacity: " << vec.capacity() << std::endl;
}

// ==================== 示例5: 复杂数据类型 ====================
void example_complex_types() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 5: Complex Data Types" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    struct Person {
        std::string name;
        int age;
        Person(const std::string& n, int a) : name(n), age(a) {}
    };

    vector<Person> people;
    
    // 使用emplace_back原地构造
    people.emplace_back("Alice", 25);
    people.emplace_back("Bob", 30);
    people.emplace_back("Charlie", 35);

    std::cout << "People count: " << people.size() << std::endl;
    
    people.for_each([](const Person& p) {
        std::cout << "  " << p.name << ", " << p.age << " years old" << std::endl;
    });
}

// ==================== 示例6: 隐式转换 ====================
void example_implicit_conversion() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 6: Implicit Conversion" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    vector<int> ts_vec;
    ts_vec.push_back(1);
    ts_vec.push_back(2);
    ts_vec.push_back(3);

    // 隐式转换到const std::vector<int>&
    const std::vector<int>& std_vec = ts_vec;
    std::cout << "Implicit conversion to const std::vector<int>&" << std::endl;
    std::cout << "Size via conversion: " << std_vec.size() << std::endl;
    
    // 显式获取拷贝
    std::vector<int> copy = ts_vec.to_vector();
    std::cout << "Explicit copy: ";
    for (int val : copy) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

// ==================== 示例7: 容量管理 ====================
void example_capacity_management() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 7: Capacity Management" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    vector<int> vec;
    
    // 预留空间
    std::cout << "Initial capacity: " << vec.capacity() << std::endl;
    vec.reserve(100);
    std::cout << "After reserve(100): " << vec.capacity() << std::endl;

    // 添加元素
    for (int i = 0; i < 50; ++i) {
        vec.push_back(i);
    }
    std::cout << "After adding 50 elements - size: " << vec.size() 
              << ", capacity: " << vec.capacity() << std::endl;

    // 改变大小
    vec.resize(30, -1);
    std::cout << "After resize(30): size = " << vec.size() << std::endl;

    // 收缩容量
    vec.shrink_to_fit();
    std::cout << "After shrink_to_fit(): capacity = " << vec.capacity() << std::endl;
}

// ==================== 示例8: 异常安全 ====================
void example_exception_safety() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Example 8: Exception Safety" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);

    // 安全的元素访问
    try {
        auto val = vec.at(0);
        std::cout << "Element at 0: " << val << std::endl;
        
        // 尝试访问越界元素
        val = vec.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "Caught out_of_range: " << e.what() << std::endl;
    }

    std::cout << "Container is still valid after exception" << std::endl;
    std::cout << "Size: " << vec.size() << std::endl;
}

// ==================== 主函数 ====================
int main() {
    std::cout << "\n"
              << "╔════════════════════════════════════════════════════╗" << std::endl
              << "║     Thread-Safe STL Vector - Usage Examples       ║" << std::endl
              << "╚════════════════════════════════════════════════════╝" << std::endl;

    example_basic_usage();
    example_lock_strategies();
    example_concurrent_operations();
    example_collection_operations();
    example_complex_types();
    example_implicit_conversion();
    example_capacity_management();
    example_exception_safety();

    std::cout << "\n"
              << "╔════════════════════════════════════════════════════╗" << std::endl
              << "║              Examples completed!                   ║" << std::endl
              << "╚════════════════════════════════════════════════════╝" << std::endl;

    return 0;
}
