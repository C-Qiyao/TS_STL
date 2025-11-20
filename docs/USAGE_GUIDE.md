# Thread-Safe STL 容器模板库

这是一个高性能的线程安全STL容器代理库，提供了简洁优雅的线程安全包装。

## 核心特性

### 1. **锁策略选择**
- **互斥锁（Mutex）**: 简单高效，适合一般场景
- **读写锁（ReadWrite）**: 允许多个读线程并发访问，适合读操作远多于写操作的场景

### 2. **STL兼容性**
- 支持隐式转换到 `const std::vector<T>&`
- 支持所有标准STL操作
- 提供 `to_vector()` 获取容器副本
- 与标准算法库兼容

### 3. **锁保护的关键接口**
以下接口自动受到锁保护，确保线程安全：
- `get()` / `set()` - 元素访问和修改
- `push_back()` / `pop_back()` - 添加/移除元素
- `clear()` - 清空容器
- `resize()` - 改变容器大小
- `reserve()` - 预留容量
- `emplace_back()` - 原地构造
- 拷贝构造和赋值操作

### 4. **高性能设计**
- 最小化锁粒度，避免不必要的同步开销
- 支持移动语义，减少内存复制
- 读操作在读写锁模式下可以并发执行
- RAII模式自动管理锁的获取和释放

## 快速开始

### 基本使用

```cpp
#include "include/ts_stl.hpp"
using namespace ts_stl;

// 使用互斥锁（默认）
vector<int> vec;

// 添加元素
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);

// 获取元素
int first = vec.get(0);  // 线程安全

// 设置元素
vec.set(1, 20);  // 线程安全

// 获取大小
size_t size = vec.size();  // 线程安全

// 清空
vec.clear();  // 线程安全
```

### 选择锁策略

```cpp
// 方法1: 使用别名（互斥锁）
vectorMutex<std::string> mutex_vec;
mutex_vec.push_back("hello");

// 方法2: 使用别名（读写锁）
vectorRW<std::string> rw_vec;
rw_vec.push_back("world");

// 方法3: 显式指定策略
vector<int, LockPolicy::ReadWrite> custom_vec;
```

### 多线程操作

```cpp
vector<int> shared_data;

std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&shared_data, i]() {
        for (int j = 0; j < 100; ++j) {
            shared_data.push_back(i * 100 + j);  // 自动线程安全
        }
    });
}

for (auto& t : threads) {
    t.join();
}

// shared_data 现在包含 400 个元素，没有竞态条件
```

### 复杂数据类型

```cpp
struct Person {
    std::string name;
    int age;
};

vector<Person> people;

// 使用 emplace_back 原地构造
people.emplace_back("Alice", 25);
people.emplace_back("Bob", 30);

// 安全访问
people.for_each([](const Person& p) {
    std::cout << p.name << " is " << p.age << std::endl;
});
```

### 隐式转换和兼容性

```cpp
vector<int> ts_vec;
ts_vec.push_back(1);
ts_vec.push_back(2);
ts_vec.push_back(3);

// 隐式转换到 const std::vector<int>&
const std::vector<int>& std_vec = ts_vec;
std::cout << std_vec.size();  // 正常使用

// 使用标准算法
int sum = 0;
std::for_each(std_vec.begin(), std_vec.end(), 
    [&sum](int val) { sum += val; });

// 或使用提供的接口
sum = 0;
ts_vec.for_each([&sum](int val) { sum += val; });
```

## API 参考

### 构造函数

| 方法 | 说明 |
|------|------|
| `ThreadSafeVector()` | 创建空容器 |
| `ThreadSafeVector(size_type count)` | 创建指定大小的容器 |
| `ThreadSafeVector(size_type count, const_reference value)` | 创建指定大小，初值相同的容器 |
| `ThreadSafeVector(const ThreadSafeVector& other)` | 拷贝构造（线程安全） |
| `ThreadSafeVector(ThreadSafeVector&& other)` | 移动构造 |

### 元素访问

| 方法 | 说明 |
|------|------|
| `const_reference get(size_type pos) const` | 获取元素（读操作，线程安全） |
| `reference get(size_type pos)` | 获取元素引用（写操作，线程安全） |
| `void set(size_type pos, const_reference value)` | 设置元素值（线程安全） |
| `const_reference at(size_type pos) const` | 安全访问（带边界检查） |
| `reference at(size_type pos)` | 非const版本 |
| `const_reference front() const` | 获取首元素 |
| `const_reference back() const` | 获取末尾元素 |

### 容量管理

| 方法 | 说明 |
|------|------|
| `size_type size() const` | 获取元素数量 |
| `size_type capacity() const` | 获取容量 |
| `bool empty() const` | 检查是否为空 |
| `void resize(size_type count)` | 改变大小 |
| `void resize(size_type count, const_reference value)` | 改变大小，用指定值填充 |
| `void reserve(size_type count)` | 预留空间 |
| `void shrink_to_fit()` | 收缩容量到大小 |

### 修改操作

| 方法 | 说明 |
|------|------|
| `void push_back(const_reference value)` | 添加元素到末尾 |
| `void push_back(T&& value)` | 移动添加元素 |
| `template<typename... Args> reference emplace_back(Args&&... args)` | 原地构造元素 |
| `void pop_back()` | 移除末尾元素 |
| `void clear()` | 清空容器 |
| `iterator insert(const_iterator pos, const_reference value)` | 插入元素 |
| `iterator erase(const_iterator pos)` | 删除元素 |
| `iterator erase(const_iterator first, const_iterator last)` | 删除范围 |

### STL 兼容性

| 方法 | 说明 |
|------|------|
| `operator const std::vector<T>&()` | 隐式转换到const引用 |
| `std::vector<T> copy() const` | 获取容器副本 |
| `std::vector<T> to_vector() const` | 同 `copy()` |
| `const std::vector<T>& ref() const` | 获取内部向量引用 |
| `T* data()` | 获取数据指针 |

### 迭代和查询

| 方法 | 说明 |
|------|------|
| `template<typename Func> void for_each(Func func) const` | 对每个元素执行操作 |
| `template<typename Predicate> const_iterator find_if(Predicate pred) const` | 条件查找 |
| `bool contains(const T& value) const` | 检查是否包含元素 |

## 性能对比

### 互斥锁 vs 读写锁

**互斥锁（Mutex）优点：**
- 实现简单，开销最小
- 适合读写操作均衡的场景
- 更好的缓存局部性

**互斥锁（Mutex）缺点：**
- 多个读线程不能并发执行

**读写锁（ReadWrite）优点：**
- 多个读线程可以并发执行
- 适合读操作远多于写操作的场景
- 在读密集型应用中性能更好

**读写锁（ReadWrite）缺点：**
- 实现复杂度更高
- 读取时开销略大
- 不适合读写操作均衡的场景

## 建议使用场景

### 使用互斥锁（Mutex）
- ✅ 生产者-消费者模式
- ✅ 读写操作均衡的场景
- ✅ 对延迟敏感的应用
- ✅ 简单的线程协调

### 使用读写锁（ReadWrite）
- ✅ 缓存系统（大量读，少量写）
- ✅ 配置管理系统
- ✅ 只读数据库
- ✅ 统计信息收集

## 编译和测试

### 使用 CMake

```bash
# 创建构建目录
mkdir build
cd build

# 生成构建文件
cmake ..

# 编译
cmake --build .

# 运行测试
ctest

# 或直接运行
./test_thread_safe_vector
./example_usage
```

### 手动编译

```bash
# 编译测试
clang++ -std=c++17 -pthread -I./include test/test_thread_safe_vector.cpp -o test

# 编译示例
clang++ -std=c++17 -pthread -I./include examples/example_usage.cpp -o example

# 运行
./test
./example
```

## 设计细节

### 锁获取策略

库使用了编译时多态（编译时条件判断）来选择锁策略：

```cpp
auto acquire_write_lock() const {
    if constexpr (Policy == LockPolicy::ReadWrite) {
        return lock_guard_.unique_lock_rw();
    } else {
        return lock_guard_.lock();
    }
}
```

这确保了零开销的抽象 - 不使用的代码路径会被编译器完全优化掉。

### RAII 模式

所有锁的获取和释放都遵循RAII原则：

```cpp
{
    auto guard = acquire_write_lock();  // 锁获取
    // 使用数据
}  // 锁自动释放
```

这保证了异常安全性 - 即使发生异常，锁也会被正确释放。

### 异常安全性

- **强异常保证**：大多数操作要么成功完成，要么不产生任何效果
- **基本异常保证**：操作失败时，容器处于有效状态
- **无异常保证**：移动操作（noexcept）

## 限制和注意事项

1. **指针返回**：`data()` 返回的指针在调用者获得后可能因其他线程的修改而失效。使用者需要自己管理。

2. **迭代器失效**：传统迭代器的概念在多线程环境中不适用。改用提供的 `for_each()` 或 `to_vector()`。

3. **原子性**：虽然单个操作是线程安全的，但多个操作的组合可能需要额外的同步。

4. **死锁风险**：在 `for_each` 的回调函数中调用 ThreadSafeVector 的其他方法可能导致死锁。

## 扩展性

这个模板库的架构支持轻松扩展到其他STL容器：

- `ThreadSafeDeque<T>`
- `list<T>`
- `ThreadSafeMap<K, V>`
- `ThreadSafeSet<T>`
- `ThreadSafeQueue<T>`
- `ThreadSafeStack<T>`

核心机制（锁管理、策略选择）可以复用。

## 许可证

MIT License

## 示例代码

详见 `examples/example_usage.cpp` 和 `test/test_thread_safe_vector.cpp`
