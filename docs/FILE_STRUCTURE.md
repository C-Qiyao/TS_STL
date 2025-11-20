# TS_STL 文件结构说明

## 项目重构

`TS_STL` 已从单一的 `ts_stl.hpp` 重构为模块化的多个头文件结构，使工程更易维护和使用。

## 新的文件结构

```
include/
├── ts_stl.hpp              # 主头文件，包含所有导出
├── ts_stl_base.hpp         # 基础组件（锁和容器基类）
├── ts_vector.hpp           # 线程安全 vector 实现
├── ts_list.hpp             # 线程安全 list 实现
└── ts_map.hpp              # 线程安全 map 实现
```

## 文件说明

### 1. `ts_stl.hpp` (主头文件)
**用途**: 用户应该只包含此文件

```cpp
#include "ts_stl.hpp"
```

此文件包含：
- 所有基础组件（通过 `ts_stl_base.hpp`）
- 所有容器实现（通过 `ts_vector.hpp` 和 `ts_list.hpp`）
- 便利的类型别名（`vectorMutex`, `listSpinLock` 等）
- 向后兼容的别名（`ThreadSafeVector`, `ThreadSafeList` 等）

### 2. `ts_stl_base.hpp` (基础组件)
**用途**: 内部使用，包含所有锁机制和容器基类

提供的组件：
- `LockPolicy` 枚举 - 锁策略定义
- `SpinLock` - 自旋锁实现
- `SpinLockGuard` - 自旋锁守卫
- `NullLockGuard` - 无锁守卫
- `UnifiedLockGuard` - 统一的锁守卫
- `LockGuard` - 锁包装器
- `container_mixin` - CRTP 基类，为容器提供共通功能

**关键特性**：
- 支持多种锁策略（互斥锁、自旋锁、读写锁、无锁）
- 使用 CRTP 模式避免代码重复
- 条件编译支持 C++17 的读写锁

### 3. `ts_vector.hpp` (线程安全 vector)
**用途**: 线程安全的 `std::vector` 实现

包含：
- `vector<T, Policy>` - 主容器类
- `vector<T, LockPolicy::LockFree>` - 零开销特化版本

**支持的锁策略**：
- `LockPolicy::Mutex` - 互斥锁（默认）
- `LockPolicy::SpinLock` - 自旋锁
- `LockPolicy::ReadWrite` - 读写锁（仅 C++17+）
- `LockPolicy::LockFree` - 无锁模式（零开销）

**典型用法**：
```cpp
ts_stl::vectorMutex<int> vec;  // 使用互斥锁
vec.push_back(42);
auto val = vec.get(0);
```

### 4. `ts_list.hpp` (线程安全 list)
**用途**: 线程安全的 `std::list` 实现

包含：
- `list<T, Policy>` - 主容器类
- `list<T, LockPolicy::LockFree>` - 零开销特化版本

**支持的锁策略**：同 vector

**典型用法**：
```cpp
ts_stl::listRW<int> list;  // 使用读写锁
list.push_back(42);
list.push_front(10);
```

## 使用方式

### 方式 1: 包含主头文件（推荐）
```cpp
#include "ts_stl.hpp"

int main() {
    ts_stl::vectorMutex<int> vec;
    vec.push_back(42);
    return 0;
}
```

### 方式 2: 包含特定头文件
```cpp
#include "ts_vector.hpp"
#include "ts_list.hpp"

int main() {
    ts_stl::vector<int, ts_stl::LockPolicy::Mutex> vec;
    ts_stl::list<int, ts_stl::LockPolicy::SpinLock> lst;
    return 0;
}
```

### 方式 3: 直接包含基础组件（高级用法）
```cpp
#include "ts_stl_base.hpp"
// 自定义使用锁机制
```

## 编译选项

项目支持 C++17 及以上版本。在 CMakeLists.txt 中配置：

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## 向后兼容性

所有旧的类型别名仍然可用：
- `ThreadSafeVector<T, Policy>`
- `ThreadSafeVectorMutex<T>`
- `ThreadSafeVectorRW<T>`
- `ThreadSafeList<T, Policy>`
- `ThreadSafeListMutex<T>`
- 等等

## 优势

1. **清晰的职责分离** - 每个文件只负责一个模块
2. **更快的编译** - 可以只包含需要的头文件
3. **易于维护** - 每个容器独立维护，减少冲突
4. **模块化扩展** - 易于添加新的容器类型（如 `deque`, `queue` 等）
5. **更好的文档** - 每个文件都有明确的用途说明

## 编译和测试

```bash
cd build
cmake ..
make
./test_thread_safe_vector
./test_thread_safe_list
./example_usage
./performance_benchmark
```

所有测试和示例都已通过编译和运行验证。
