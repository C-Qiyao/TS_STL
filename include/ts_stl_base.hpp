#pragma once

#ifndef TS_STL_BASE_HPP
#define TS_STL_BASE_HPP

#include <memory>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <atomic>

// C++ 版本检查
#if __cplusplus < 201703L
    #error "TS_STL requires C++17 or later"
#endif

// 条件编译：仅在C++17及以上时启用读写锁
#if __cplusplus >= 201703L
    #include <shared_mutex>
    #define TS_STL_SUPPORT_RW_LOCK 1
#else
    #define TS_STL_SUPPORT_RW_LOCK 0
#endif

namespace ts_stl {

// 锁策略枚举
enum class LockPolicy {
    Mutex,      // 互斥锁（所有C++版本都支持）
    SpinLock,   // 自旋锁（轻量级，适合短临界区）
    LockFree,   // 无锁（极限性能，需要单线程或外部同步）
#if TS_STL_SUPPORT_RW_LOCK
    ReadWrite   // 读写锁（仅C++17+支持）
#endif
};

/**
 * @brief 简单的自旋锁实现 - 适合短临界区
 * 
 * 自旋锁通过忙等待而非睡眠来获取锁。
 * 适用于：
 * - 临界区非常短（< 几微秒）
 * - 多核系统上的竞争不激烈的场景
 * 
 * 避免用于：
 * - 长临界区（会浪费 CPU）
 * - 单核系统（没有优势）
 * - 高竞争场景（忙等待效率低）
 */
class SpinLock {
public:
    SpinLock() = default;
    
    // 不可复制不可移动
    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;

    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // 忙等待，直到获得锁
            // 可以添加 pause 指令来减轻 CPU 压力
            #ifdef __x86_64__
                asm volatile("pause");
            #elif defined(__aarch64__)
                asm volatile("yield");
            #endif
        }
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

/**
 * @brief 自旋锁的 unique_lock 兼容包装
 */
class SpinLockGuard {
public:
    explicit SpinLockGuard(SpinLock& lock) : lock_(lock) {
        lock_.lock();
        owns_lock_ = true;
    }

    ~SpinLockGuard() {
        if (owns_lock_) {
            lock_.unlock();
        }
    }

    // 移动语义
    SpinLockGuard(SpinLockGuard&& other) noexcept 
        : lock_(other.lock_), owns_lock_(other.owns_lock_) {
        other.owns_lock_ = false;
    }

    SpinLockGuard& operator=(SpinLockGuard&& other) noexcept {
        if (this != &other) {
            if (owns_lock_) {
                lock_.unlock();
            }
            owns_lock_ = other.owns_lock_;
            other.owns_lock_ = false;
        }
        return *this;
    }

    // 不可复制
    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;

private:
    SpinLock& lock_;
    bool owns_lock_ = false;
};

/**
 * @brief 空锁守卫 - 无锁策略
 * 
 * 用于极限性能场景。完全不获取任何锁。
 * 使用场景：
 * - 单线程代码
 * - 外部已做同步的代码
 * - 初始化/准备阶段
 * 
 * ⚠️ 警告：使用此策略时，用户需确保线程安全性！
 */
class NullLockGuard {
public:
    NullLockGuard() = default;
    
    // 移动语义（空实现）
    NullLockGuard(NullLockGuard&&) noexcept {}
    
    NullLockGuard& operator=(NullLockGuard&&) noexcept {
        return *this;
    }

    // 不可复制
    NullLockGuard(const NullLockGuard&) = delete;
    NullLockGuard& operator=(const NullLockGuard&) = delete;
};

/**
 * @brief 统一的锁守卫类型 - 支持多种锁机制
 */
class UnifiedLockGuard {
public:
    // 用于 std::mutex
    UnifiedLockGuard(std::unique_lock<std::mutex>&& lock) 
        : spinlock_(nullptr), lock_type_(0), mutex_lock_(std::move(lock)) {}
    
    // 用于 SpinLock
    UnifiedLockGuard(SpinLock& lock) 
        : spinlock_(&lock), lock_type_(1) {
        spinlock_->lock();
    }

    // 用于 NullLockGuard (无锁)
    UnifiedLockGuard(NullLockGuard&&) 
        : spinlock_(nullptr), lock_type_(3) {}

#if TS_STL_SUPPORT_RW_LOCK
    // 用于 shared_mutex 写锁
    UnifiedLockGuard(std::unique_lock<std::shared_mutex>&& lock)
        : spinlock_(nullptr), lock_type_(2), rw_lock_(std::move(lock)) {}
#endif

    ~UnifiedLockGuard() {
        if (lock_type_ == 1 && spinlock_) {
            spinlock_->unlock();
        }
    }

    // 移动语义
    UnifiedLockGuard(UnifiedLockGuard&& other) noexcept 
        : spinlock_(other.spinlock_), lock_type_(other.lock_type_) {
        if (other.lock_type_ == 0) {
            mutex_lock_ = std::move(other.mutex_lock_);
        }
#if TS_STL_SUPPORT_RW_LOCK
        else if (other.lock_type_ == 2) {
            rw_lock_ = std::move(other.rw_lock_);
        }
#endif
        other.spinlock_ = nullptr;
        other.lock_type_ = -1;
    }

    UnifiedLockGuard& operator=(UnifiedLockGuard&& other) noexcept {
        if (this != &other) {
            this->~UnifiedLockGuard();
            spinlock_ = other.spinlock_;
            lock_type_ = other.lock_type_;
            if (other.lock_type_ == 0) {
                mutex_lock_ = std::move(other.mutex_lock_);
            }
#if TS_STL_SUPPORT_RW_LOCK
            else if (other.lock_type_ == 2) {
                rw_lock_ = std::move(other.rw_lock_);
            }
#endif
            other.spinlock_ = nullptr;
            other.lock_type_ = -1;
        }
        return *this;
    }

    // 不可复制
    UnifiedLockGuard(const UnifiedLockGuard&) = delete;
    UnifiedLockGuard& operator=(const UnifiedLockGuard&) = delete;

private:
    SpinLock* spinlock_;
    int lock_type_;
    std::unique_lock<std::mutex> mutex_lock_;

#if TS_STL_SUPPORT_RW_LOCK
    std::unique_lock<std::shared_mutex> rw_lock_;
#endif
};

/**
 * @brief 锁包装器 - 统一处理互斥锁、自旋锁和读写锁
 */
class LockGuard {
public:
    explicit LockGuard(LockPolicy policy = LockPolicy::Mutex) 
        : policy_(policy) {
        if (policy == LockPolicy::Mutex) {
            mutex_ = std::make_unique<std::mutex>();
        }
        else if (policy == LockPolicy::SpinLock) {
            spinlock_ = std::make_unique<SpinLock>();
        }
#if TS_STL_SUPPORT_RW_LOCK
        else if (policy == LockPolicy::ReadWrite) {
            shared_mutex_ = std::make_unique<std::shared_mutex>();
        }
#endif
    }

    // 获取写锁 - 返回 UnifiedLockGuard 以支持不同锁类型
    UnifiedLockGuard write_lock() const {
        if (policy_ == LockPolicy::Mutex) {
            return UnifiedLockGuard(std::unique_lock<std::mutex>(*mutex_));
        }
        else if (policy_ == LockPolicy::SpinLock) {
            return UnifiedLockGuard(*spinlock_);
        }
        else if (policy_ == LockPolicy::LockFree) {
            return UnifiedLockGuard(NullLockGuard());
        }
#if TS_STL_SUPPORT_RW_LOCK
        throw std::logic_error("Cannot acquire write_lock on ReadWrite policy");
#else
        throw std::logic_error("ReadWrite lock is not supported in this C++ version");
#endif
    }

    // 获取互斥锁
    UnifiedLockGuard lock() const {
        return write_lock();
    }
    
#if TS_STL_SUPPORT_RW_LOCK
    // 获取读锁（只在读写锁时有效）
    std::shared_lock<std::shared_mutex> read_lock() const {
        if (policy_ == LockPolicy::ReadWrite) {
            return std::shared_lock<std::shared_mutex>(*shared_mutex_);
        }
        throw std::logic_error("Cannot acquire read_lock on Mutex policy");
    }

    // 获取互斥锁的写锁（用于读写锁）
    std::unique_lock<std::shared_mutex> unique_lock_rw() const {
        if (policy_ == LockPolicy::ReadWrite) {
            return std::unique_lock<std::shared_mutex>(*shared_mutex_);
        }
        throw std::logic_error("Cannot acquire unique_lock_rw on Mutex policy");
    }
#endif

    LockPolicy policy() const { return policy_; }

private:
    LockPolicy policy_;
    std::unique_ptr<std::mutex> mutex_;
    std::unique_ptr<SpinLock> spinlock_;
#if TS_STL_SUPPORT_RW_LOCK
    std::unique_ptr<std::shared_mutex> shared_mutex_;
#endif
};

/**
 * @brief CRTP 基类 - 为 vector 和 list 提供共用功能
 * @tparam Derived 派生类（vector 或 list）
 * @tparam T 元素类型
 * @tparam Policy 锁策略
 * 
 * 使用 CRTP 模式消除代码重复，同时保持零开销抽象
 */
template <typename Derived, typename T, LockPolicy Policy>
class container_mixin {
protected:
    LockGuard lock_guard_;

    // CRTP：获取派生类引用
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }

    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }

    // 获取写锁
    auto acquire_write_lock() const {
        if constexpr (Policy == LockPolicy::Mutex) {
            return lock_guard_.lock();
        }
        else if constexpr (Policy == LockPolicy::SpinLock) {
            return lock_guard_.lock();
        }
        else if constexpr (Policy == LockPolicy::LockFree) {
            return lock_guard_.lock();  // 返回空锁守卫
        }
#if TS_STL_SUPPORT_RW_LOCK
        else if constexpr (Policy == LockPolicy::ReadWrite) {
            return lock_guard_.unique_lock_rw();
        }
#endif
    }

    // 获取读锁
    auto acquire_read_lock() const {
        if constexpr (Policy == LockPolicy::LockFree) {
            return lock_guard_.lock();  // 返回空锁守卫
        }
        else if constexpr (Policy == LockPolicy::SpinLock) {
            return lock_guard_.lock();
        }
#if TS_STL_SUPPORT_RW_LOCK
        else if constexpr (Policy == LockPolicy::ReadWrite) {
            return lock_guard_.read_lock();
        }
#endif
        else {
            return lock_guard_.lock();
        }
    }

public:
    container_mixin() : lock_guard_(Policy) {}

    // ==================== 通用线程不安全接口 ====================

    auto unsafe_size() const {
        return derived().data_.size();
    }

    bool unsafe_empty() const {
        return derived().data_.empty();
    }

    auto& unsafe_ref() {
        return derived().data_;
    }

    const auto& unsafe_ref() const {
        return derived().data_;
    }

    // ==================== 通用手动锁控制接口 ====================

    auto acquire_write_guard() const {
        return acquire_write_lock();
    }

#if TS_STL_SUPPORT_RW_LOCK
    auto acquire_read_guard() const {
        return acquire_read_lock();
    }
#endif

    template <typename Func>
    void with_write_lock(Func func) const {
        auto guard = acquire_write_lock();
        func(derived());
    }

#if TS_STL_SUPPORT_RW_LOCK
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(derived());
    }
#endif

    // ==================== 通用迭代和查询接口 ====================

    template <typename Func>
    void for_each(Func func) const {
        auto guard = acquire_read_lock();
        std::for_each(derived().data_.begin(), derived().data_.end(), func);
    }

    template <typename Predicate>
    auto find_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::find_if(derived().data_.begin(), derived().data_.end(), pred);
    }

    bool contains(const T& value) const {
        auto guard = acquire_read_lock();
        return std::find(derived().data_.begin(), derived().data_.end(), value) != derived().data_.end();
    }

    auto count(const T& value) const {
        auto guard = acquire_read_lock();
        return static_cast<decltype(derived().data_.size())>(
            std::count(derived().data_.begin(), derived().data_.end(), value)
        );
    }

    // 获取容器拷贝
    auto copy() const {
        auto guard = acquire_read_lock();
        return derived().data_;
    }

    virtual ~container_mixin() = default;
};

} // namespace ts_stl

#endif // TS_STL_BASE_HPP
