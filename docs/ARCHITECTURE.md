# TS_STL 架构总结

## 项目结构

```
TS_STL/
├── include/
│   └── ts_stl.hpp                   # 核心库实现（~1146行）
├── test/
│   ├── test_thread_safe_vector.cpp  # Vector 测试套件（350行）
│   ├── test_advanced_features.cpp   # 高级功能测试（320行）
│   └── test_thread_safe_list.cpp    # List 测试套件（300行）
├── examples/
│   └── example_usage.cpp            # 使用示例（245行）
├── docs/
│   ├── CRTP_OPTIMIZATION.md         # CRTP 优化详解
│   ├── REFACTORING_NOTES.md         # 重构方案对比
│   ├── README.md                    # 项目概览
│   ├── USAGE_GUIDE.md               # API 使用指南
│   ├── ADVANCED_FEATURES.md         # 高级功能指南
│   └── PROJECT_SUMMARY.md           # 项目统计
└── CMakeLists.txt                   # 构建配置
```

## 核心架构

### 分层设计

```
┌─────────────────────────────────────────────┐
│         用户代码                             │
│  vectorMutex<int>                 │
│  listRW<std::string>              │
└────────────────┬────────────────────────────┘
                 │
┌────────────────▼────────────────────────────┐
│         容器类（派生）                       │
│  ┌─────────────────────┐                   │
│  │ vector<T> │ ◄─────┐           │
│  └─────────────────────┘       │ CRTP     │
│  ┌─────────────────────┐       │ 继承     │
│  │ list<T>   │ ◄─────┤          │
│  └─────────────────────┘       │           │
│                                 │           │
└────────────────────────────────┼───────────┘
                                 │
┌────────────────────────────────▼───────────┐
│     CRTP 基类（共用功能）                  │
│  ThreadSafeContainerMixin<Derived, T, P>   │
│                                            │
│  • 锁管理（acquire_write_lock等）          │
│  • 线程不安全接口（unsafe_*）              │
│  • 手动锁控制（with_write_lock等）         │
│  • 迭代查询（for_each, contains等）       │
└────────────────┬────────────────────────────┘
                 │
┌────────────────▼────────────────────────────┐
│         底层原语                             │
│  • LockGuard：统一的锁管理                 │
│  • LockPolicy：Mutex/ReadWrite 选择         │
│  • std::mutex, std::shared_mutex            │
└─────────────────────────────────────────────┘
```

## 类型系统

### 核心类型

| 类型 | 说明 | 用途 |
|------|------|------|
| `LockPolicy` | 枚举：Mutex 或 ReadWrite | 编译时选择锁策略 |
| `LockGuard` | 统一的锁包装器 | 管理互斥锁和读写锁 |
| `ThreadSafeContainerMixin<D,T,P>` | CRTP 基类 | 提供共用功能 |
| `vector<T,P>` | Vector 代理 | 线程安全的 std::vector |
| `list<T,P>` | List 代理 | 线程安全的 std::list |

### 类型别名

```cpp
// Vector 类型别名
using vectorMutex<T> = vector<T, LockPolicy::Mutex>;
using vectorRW<T> = vector<T, LockPolicy::ReadWrite>;

// List 类型别名
using listMutex<T> = list<T, LockPolicy::Mutex>;
using listRW<T> = list<T, LockPolicy::ReadWrite>;
```

## 功能模块

### 1. 锁管理模块

**LockGuard 类**
```
┌─────────────────────────────────┐
│ LockGuard(LockPolicy)           │
├─────────────────────────────────┤
│ Policy:                         │
│ • Mutex → std::mutex            │
│ • ReadWrite → std::shared_mutex │
├─────────────────────────────────┤
│ 方法：                          │
│ • write_lock()                  │
│ • read_lock()                   │
│ • unique_lock_rw()              │
└─────────────────────────────────┘
```

### 2. CRTP 基类模块

**ThreadSafeContainerMixin**

提供的接口分为四类：

1. **锁管理接口**（2个方法）
   - `acquire_write_lock()` - 获取写锁
   - `acquire_read_lock()` - 获取读锁

2. **线程不安全接口**（4个方法）
   - `unsafe_size()` - 获取大小（无锁）
   - `unsafe_empty()` - 检查是否为空（无锁）
   - `unsafe_ref()` - 获取容器引用（无锁）

3. **手动锁控制**（4个方法）
   - `acquire_write_guard()` - 获取写锁 guard
   - `acquire_read_guard()` - 获取读锁 guard（C++17+）
   - `with_write_lock(func)` - 在写锁下执行
   - `with_read_lock(func)` - 在读锁下执行（C++17+）

4. **迭代和查询**（5个方法）
   - `for_each(func)` - 遍历元素
   - `find_if(predicate)` - 条件查找
   - `contains(value)` - 包含检查
   - `count(value)` - 元素计数
   - `copy()` - 获取拷贝

### 3. Vector 特有功能

**元素访问**
- `get(index)` - 安全获取元素
- `set(index, value)` - 安全设置元素
- `at(index)` - 带边界检查的访问
- `front()`, `back()`

**容量管理**（Vector 特有）
- `capacity()` - 获取容量
- `reserve(count)` - 预留空间
- `shrink_to_fit()` - 收缩容量
- `resize(count)` - 改变大小

**修改操作**
- `push_back(value)` - 添加元素
- `emplace_back(args...)` - 原地构造
- `pop_back()` - 删除末尾
- `insert(pos, value)`, `erase(pos)` - 位置操作

### 4. List 特有功能

**首尾操作**（List 特有）
- `push_front(value)`, `pop_front()` - 首部操作
- `set_front(value)`, `set_back(value)` - 设置首尾

**List 特有的修改**（List 特有）
- `remove(value)` - 移除所有指定值
- `remove_if(predicate)` - 条件移除
- `reverse()` - 反转
- `sort()` - 排序

## 线程安全保证

### 操作分类

```
┌──────────────────────────────────┐
│     操作分类                     │
├──────────────────────────────────┤
│ 1. 安全操作（自动加锁）          │
│    • get(), set(), push_back()   │
│    • for_each(), contains()      │
│                                  │
│ 2. 线程不安全操作（无保护）      │
│    • unsafe_ref(), unsafe_size() │
│    • 用于已加锁或单线程场景      │
│                                  │
│ 3. 手动控制操作（灵活加锁）      │
│    • with_write_lock(func)       │
│    • acquire_write_guard()       │
│    • 用于需要原子多操作的场景    │
└──────────────────────────────────┘
```

### 锁策略

**Mutex 策略**（简单，适合通用场景）
```cpp
vectorMutex<int> vec;  // 所有操作用互斥锁保护
```

**ReadWrite 策略**（复杂，适合读多写少）
```cpp
vectorRW<int> vec;  // 读操作可并发，写操作排斥
```

## 代码复用率

### 之前（无基类）
```
ThreadSafeVector: 100% 实现
ThreadSafeList: 100% 实现
重复代码: ~30% (~150行)
总计: 750行
```

### 之后（CRTP 优化）
```
ThreadSafeContainerMixin: 120行共用代码
ThreadSafeVector: 350行（包含特有操作）
ThreadSafeList: 200行（包含特有操作）
重复率: 0%
总计: 670行
节省: 80行 + 0% 重复
```

## 编译和运行

### 编译
```bash
cd build
cmake ..
cmake --build .
```

### 测试
```bash
./test_thread_safe_vector     # Vector 基础测试（10个测试组）
./test_advanced_features      # 高级功能测试（5个测试组）
./test_thread_safe_list       # List 测试（10个测试组）
```

### 性能特征

| 操作 | 时间复杂度 | 锁粒度 | 备注 |
|------|-----------|--------|------|
| get/set | O(1) | 细 | 单操作加锁 |
| push_back | O(1)* | 细 | *Vector 平摊 |
| 批量操作 | O(n) | 粗 | with_write_lock 优化 |
| for_each | O(n) | 粗 | 遍历期间持有锁 |

## 最佳实践

### 1. 选择合适的锁策略

```cpp
// 场景1：读写均衡 → 用 Mutex
vectorMutex<int> vec1;

// 场景2：读多写少 → 用 ReadWrite（C++17+）
vectorRW<int> vec2;
```

### 2. 使用 with_write_lock 进行原子多操作

```cpp
// ❌ 不好：三个独立的加锁操作
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);

// ✅ 好：一次加锁，三个操作
vec.with_write_lock([](auto& v) {
    v.unsafe_push_back(1);
    v.unsafe_push_back(2);
    v.unsafe_push_back(3);
});
```

### 3. 性能敏感时使用 unsafe 接口

```cpp
// 已确保单线程或已加锁
auto size = vec.unsafe_size();  // 无锁，最快
auto ref = vec.unsafe_ref();    // 获取原始容器
```

## 扩展性

### 添加新容器

要添加 `ThreadSafeDeque`：

```cpp
template <typename T, LockPolicy Policy = LockPolicy::Mutex>
class ThreadSafeDeque 
    : public ThreadSafeContainerMixin<ThreadSafeDeque<T, Policy>, T, Policy> {
    
    std::deque<T> data_;  // 自动继承所有通用功能！
    
    // 仅添加 deque 特有的操作
    void push_back(const T& v) { 
        auto guard = acquire_write_lock();
        data_.push_back(v); 
    }
    // ...
};

using ThreadSafeDequeM utex<T> = ThreadSafeDeque<T, LockPolicy::Mutex>;
```

**优势**：
- 代码复用率 70%+
- 自动获得所有通用功能
- 无需重复实现锁管理

## 文档资源

- **README.md** - 项目概览和快速开始
- **USAGE_GUIDE.md** - 详细的 API 文档
- **ADVANCED_FEATURES.md** - 高级用法指南
- **CRTP_OPTIMIZATION.md** - CRTP 设计详解
- **PROJECT_SUMMARY.md** - 项目统计和完成度

## 验证清单

- ✅ 所有 25+ 个单元测试通过
- ✅ 无编译警告
- ✅ 零开销抽象（无虚函数）
- ✅ 完整的异常安全保证
- ✅ C++17 标准兼容
- ✅ CRTP 优化完成（0% 代码重复）
- ✅ 文档完整（6个文档文件）
