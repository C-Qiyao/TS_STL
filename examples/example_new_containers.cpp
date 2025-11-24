#include <iostream>
#include "ts_stl.hpp"

using namespace ts_stl;

// ==================== Set 示例 ====================
void example_set() {
    std::cout << "\n=== Set 示例 ===" << std::endl;
    
    // 创建线程安全的集合
    setMutex<int> numbers;
    
    // 插入元素
    numbers.insert(42);
    numbers.insert(17);
    numbers.insert(99);
    numbers.insert(42);  // 重复元素被忽略
    
    std::cout << "Set 大小: " << numbers.size() << std::endl;
    
    // 检查元素
    if (numbers.contains(42)) {
        std::cout << "集合包含 42" << std::endl;
    }
    
    // 遍历元素
    std::cout << "集合中的元素: ";
    numbers.for_each([](int n) {
        std::cout << n << " ";
    });
    std::cout << std::endl;
    
    // 删除元素
    numbers.erase(17);
    std::cout << "删除 17 后，大小: " << numbers.size() << std::endl;
}

// ==================== Unordered Set 示例 ====================
void example_unordered_set() {
    std::cout << "\n=== Unordered Set 示例 ===" << std::endl;
    
    // 创建线程安全的无序集合
    unordered_setMutex<std::string> tags;
    
    // 插入元素
    tags.insert("C++");
    tags.insert("Python");
    tags.insert("Rust");
    tags.insert("Go");
    
    std::cout << "标签数量: " << tags.size() << std::endl;
    
    // 检查元素
    std::cout << "包含 'Python': " << (tags.contains("Python") ? "是" : "否") << std::endl;
    std::cout << "包含 'Java': " << (tags.contains("Java") ? "是" : "否") << std::endl;
    
    // 遍历元素
    std::cout << "所有标签: ";
    tags.for_each([](const std::string& tag) {
        std::cout << tag << " ";
    });
    std::cout << std::endl;
}

// ==================== Deque 示例 ====================
void example_deque() {
    std::cout << "\n=== Deque 示例 ===" << std::endl;
    
    // 创建线程安全的双端队列
    dequeMutex<int> queue;
    
    // 从两端插入元素
    queue.push_back(10);
    queue.push_back(20);
    queue.push_back(30);
    queue.push_front(0);
    queue.push_front(-10);
    
    std::cout << "双端队列大小: " << queue.size() << std::endl;
    std::cout << "队列前端: " << queue.front() << std::endl;
    std::cout << "队列后端: " << queue.back() << std::endl;
    
    // 遍历元素
    std::cout << "队列内容: ";
    queue.for_each([](int n) {
        std::cout << n << " ";
    });
    std::cout << std::endl;
    
    // 从两端移除元素
    queue.pop_front();
    queue.pop_back();
    std::cout << "移除前后端元素后，大小: " << queue.size() << std::endl;
}

// ==================== 不同的锁策略 ====================
void example_lock_strategies() {
    std::cout << "\n=== 锁策略示例 ===" << std::endl;
    
    // 使用 Mutex 锁
    setMutex<int> set1;
    
    // 使用读写锁（C++17+）
    setRW<int> set2;
    
    // 使用自旋锁
    setSpinLock<int> set3;
    
    // 使用无锁版本（低开销）
    setLockFree<int> set4;
    
    for (int i = 1; i <= 5; ++i) {
        set1.insert(i);
        set2.insert(i);
        set3.insert(i);
        set4.insert(i);
    }
    
    std::cout << "Mutex Set 大小: " << set1.size() << std::endl;
    std::cout << "ReadWrite Set 大小: " << set2.size() << std::endl;
    std::cout << "SpinLock Set 大小: " << set3.size() << std::endl;
    std::cout << "LockFree Set 大小: " << set4.size() << std::endl;
}

// ==================== 高级操作 ====================
void example_advanced_operations() {
    std::cout << "\n=== 高级操作示例 ===" << std::endl;
    
    setMutex<int> numbers;
    for (int i = 1; i <= 10; ++i) {
        numbers.insert(i * i);
    }
    
    // 使用 count_if 统计元素
    auto count = numbers.count_if([](int n) {
        return n > 30;
    });
    std::cout << "大于 30 的平方数: " << count << std::endl;
    
    // 使用 contains 检查元素
    if (numbers.contains(64)) {
        std::cout << "集合包含 64（8的平方）" << std::endl;
    }
    
    // Deque 的遍历操作
    dequeMutex<int> dq;
    dq.push_back(1);
    dq.push_back(2);
    dq.push_back(3);
    dq.push_back(4);
    dq.push_back(5);
    
    // 统计 Deque 中大于2的元素
    auto gt2 = dq.count_if([](int val) {
        return val > 2;
    });
    std::cout << "Deque 中大于 2 的元素个数: " << gt2 << std::endl;
}

int main() {
    std::cout << "TS_STL 新容器使用示例" << std::endl;
    std::cout << "=====================" << std::endl;
    
    example_set();
    example_unordered_set();
    example_deque();
    example_lock_strategies();
    example_advanced_operations();
    
    std::cout << "\n✓ 所有示例执行完毕" << std::endl;
    return 0;
}
