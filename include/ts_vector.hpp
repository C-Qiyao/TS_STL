#pragma once

#ifndef TS_VECTOR_HPP
#define TS_VECTOR_HPP

#include <vector>
#include <stdexcept>
#include <algorithm>

#include "ts_stl_base.hpp"

namespace ts_stl {

/**
 * @brief 线程安全的Vector代理类
 * @tparam T 元素类型
 * @tparam Policy 锁策略（默认使用互斥锁）
 */
template <typename T, LockPolicy Policy = LockPolicy::Mutex>
class vector : public container_mixin<vector<T, Policy>, T, Policy> {
private:
    friend class container_mixin<vector<T, Policy>, T, Policy>;

    std::vector<T> data_;

    // 获取锁 - 使用基类提供的方法
    using Base = container_mixin<vector<T, Policy>, T, Policy>;
    using Base::acquire_write_lock;
    using Base::acquire_read_lock;

public:
    // 为基类提供必要的类型信息
    using Container = std::vector<T>;
    
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;
    using difference_type = typename std::vector<T>::difference_type;

    // 构造函数
    vector() : Base() {}

    explicit vector(size_type count) 
        : Base() {
        data_.resize(count);
    }

    vector(size_type count, const_reference value)
        : Base() {
        data_.assign(count, value);
    }

    template <typename InputIt>
    vector(InputIt first, InputIt last)
        : Base() {
        data_.assign(first, last);
    }

    // 拷贝构造函数 - 需要锁保护
    vector(const vector& other) : Base() {
        auto guard = acquire_write_lock();
        data_ = other.data_;
    }

    // 移动构造函数
    vector(vector&& other) noexcept 
        : Base() {
        auto guard = acquire_write_lock();
        data_ = std::move(other.data_);
    }

    // 赋值操作符
    vector& operator=(const vector& other) {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = other.data_;
        }
        return *this;
    }

    // 移动赋值操作符
    vector& operator=(vector&& other) noexcept {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = std::move(other.data_);
        }
        return *this;
    }

    // ==================== 元素访问 ====================
    
    /**
     * @brief 获取指定位置的元素（支持隐式转换）
     */
    const_reference get(size_type pos) const {
        auto guard = acquire_read_lock();
        if (pos >= data_.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data_[pos];
    }

    /**
     * @brief 获取指定位置的元素（非const版本）
     */
    reference get(size_type pos) {
        auto guard = acquire_write_lock();
        if (pos >= data_.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data_[pos];
    }

    /**
     * @brief 设置指定位置的元素
     */
    void set(size_type pos, const_reference value) {
        auto guard = acquire_write_lock();
        if (pos >= data_.size()) {
            throw std::out_of_range("Index out of range");
        }
        data_[pos] = value;
    }

    /**
     * @brief at - 安全的元素访问
     */
    const_reference at(size_type pos) const {
        auto guard = acquire_read_lock();
        return data_.at(pos);
    }

    reference at(size_type pos) {
        auto guard = acquire_write_lock();
        return data_.at(pos);
    }

    /**
     * @brief operator[] - 支持数组风格访问 (const版本)
     * @note 无边界检查，用于性能关键代码
     */
    const_reference operator[](size_type pos) const {
        auto guard = acquire_read_lock();
        return data_[pos];
    }

    /**
     * @brief operator[] - 支持数组风格访问 (非const版本)
     * @note 无边界检查，用于性能关键代码
     */
    reference operator[](size_type pos) {
        auto guard = acquire_write_lock();
        return data_[pos];
    }

    /**
     * @brief 获取第一个元素
     */
    const_reference front() const {
        auto guard = acquire_read_lock();
        if (data_.empty()) {
            throw std::out_of_range("Vector is empty");
        }
        return data_.front();
    }

    /**
     * @brief 获取最后一个元素
     */
    const_reference back() const {
        auto guard = acquire_read_lock();
        if (data_.empty()) {
            throw std::out_of_range("Vector is empty");
        }
        return data_.back();
    }

    // ==================== 容量管理 ====================

    /**
     * @brief 获取容器大小
     */
    size_type size() const {
        auto guard = acquire_read_lock();
        return data_.size();
    }

    /**
     * @brief 获取容器容量
     */
    size_type capacity() const {
        auto guard = acquire_read_lock();
        return data_.capacity();
    }

    /**
     * @brief 检查容器是否为空
     */
    bool empty() const {
        auto guard = acquire_read_lock();
        return data_.empty();
    }

    /**
     * @brief 重新分配容器大小 - 需要锁保护
     */
    void resize(size_type count) {
        auto guard = acquire_write_lock();
        data_.resize(count);
    }

    /**
     * @brief 重新分配容器大小，新元素使用指定值 - 需要锁保护
     */
    void resize(size_type count, const_reference value) {
        auto guard = acquire_write_lock();
        data_.resize(count, value);
    }

    /**
     * @brief 预留空间 - 需要锁保护
     */
    void reserve(size_type count) {
        auto guard = acquire_write_lock();
        data_.reserve(count);
    }

    /**
     * @brief 收缩容量 - 需要锁保护
     */
    void shrink_to_fit() {
        auto guard = acquire_write_lock();
        data_.shrink_to_fit();
    }

    // ==================== 修改操作 ====================

    /**
     * @brief 添加元素到末尾
     */
    void push_back(const_reference value) {
        auto guard = acquire_write_lock();
        data_.push_back(value);
    }

    /**
     * @brief 移动语义添加元素到末尾
     */
    void push_back(T&& value) {
        auto guard = acquire_write_lock();
        data_.push_back(std::move(value));
    }

    /**
     * @brief 原地构造元素到末尾
     */
    template <typename... Args>
    reference emplace_back(Args&&... args) {
        auto guard = acquire_write_lock();
        return data_.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief 移除末尾元素
     */
    void pop_back() {
        auto guard = acquire_write_lock();
        if (data_.empty()) {
            throw std::out_of_range("Cannot pop from empty vector");
        }
        data_.pop_back();
    }

    /**
     * @brief 清空容器 - 需要锁保护
     */
    void clear() {
        auto guard = acquire_write_lock();
        data_.clear();
    }

    /**
     * @brief 在指定位置插入元素
     */
    iterator insert(const_iterator pos, const_reference value) {
        auto guard = acquire_write_lock();
        return data_.insert(pos, value);
    }

    /**
     * @brief 在指定位置移除元素
     */
    iterator erase(const_iterator pos) {
        auto guard = acquire_write_lock();
        return data_.erase(pos);
    }

    /**
     * @brief 在指定范围内移除元素
     */
    iterator erase(const_iterator first, const_iterator last) {
        auto guard = acquire_write_lock();
        return data_.erase(first, last);
    }

    // ==================== STL兼容性 ====================

    /**
     * @brief 获取原始数据指针（需要小心使用）
     */
    T* data() {
        // 不加锁，因为返回指针后外部可能需要长期持有
        // 使用者需要自己管理并发安全
        return data_.data();
    }

    const T* data() const {
        return data_.data();
    }

    /**
     * @brief 隐式转换到std::vector（仅在读操作时使用）
     */
    operator const std::vector<T>&() const {
        return data_;
    }

    /**
     * @brief 获取内部vector的拷贝
     */
    std::vector<T> copy() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    /**
     * @brief 获取内部vector的引用（提供给需要直接访问的场景）
     * 注意：调用者需要自己处理同步
     */
    const std::vector<T>& ref() const {
        return data_;
    }

    // ==================== 迭代器 ====================

    /**
     * @brief 安全的元素遍历 - 返回vector拷贝
     */
    std::vector<T> to_vector() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    /**
     * @brief 对每个元素执行操作
     */
    template <typename Func>
    void for_each(Func func) const {
        auto guard = acquire_read_lock();
        std::for_each(data_.begin(), data_.end(), func);
    }

    /**
     * @brief 条件查找
     */
    template <typename Predicate>
    const_iterator find_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::find_if(data_.begin(), data_.end(), pred);
    }

    /**
     * @brief 判断是否包含某个元素
     */
    bool contains(const T& value) const {
        auto guard = acquire_read_lock();
        return std::find(data_.begin(), data_.end(), value) != data_.end();
    }

    // ==================== 比较操作 ====================

    bool operator==(const vector& other) const {
        auto guard = acquire_read_lock();
        return data_ == other.data_;
    }

    bool operator!=(const vector& other) const {
        return !(*this == other);
    }

    // ==================== 线程不安全接口 ====================
    // 注意：以下接口不进行锁保护，仅在确保外部同步或单线程场景下使用

    /**
     * @brief 获取内部vector的非const引用（线程不安全）
     * 
     * 使用场景：
     * - 需要批量操作且在外部已加锁
     * - 需要使用STL算法但已确保单线程访问
     * 
     * 危险：调用者需要完全负责线程安全，任何并发访问都会导致未定义行为
     */
    std::vector<T>& unsafe_ref() {
        return data_;
    }

    const std::vector<T>& unsafe_ref() const {
        return data_;
    }

    /**
     * @brief 线程不安全的元素访问
     * 
     * 性能最好，但完全不进行线程安全检查
     * 仅在外部已加锁或确保单线程时使用
     */
    T& unsafe_at(size_type pos) {
        return data_[pos];
    }

    const T& unsafe_at(size_type pos) const {
        return data_[pos];
    }

    /**
     * @brief 线程不安全的push_back
     * 
     * 在已获取写锁的情况下使用，避免二次加锁
     */
    void unsafe_push_back(const_reference value) {
        data_.push_back(value);
    }

    void unsafe_push_back(T&& value) {
        data_.push_back(std::move(value));
    }

    /**
     * @brief 线程不安全的clear
     * 
     * 在已获取写锁的情况下使用
     */
    void unsafe_clear() {
        data_.clear();
    }

    /**
     * @brief 线程不安全的resize
     * 
     * 在已获取写锁的情况下使用
     */
    void unsafe_resize(size_type count) {
        data_.resize(count);
    }

    void unsafe_resize(size_type count, const_reference value) {
        data_.resize(count, value);
    }

    /**
     * @brief 线程不安全的size获取
     * 
     * 在已获取读锁或确保单线程时使用
     */
    size_type unsafe_size() const {
        return data_.size();
    }

    /**
     * @brief 线程不安全的empty检查
     * 
     * 在已获取读锁或确保单线程时使用
     */
    bool unsafe_empty() const {
        return data_.empty();
    }

    // ==================== 手动锁控制接口 ====================
    // 用于外部需要手动控制锁的场景

    /**
     * @brief 获取写锁guard供外部使用
     * 
     * 使用示例：
     * ```cpp
     * auto guard = vec.acquire_write_guard();
     * vec.unsafe_push_back(value1);
     * vec.unsafe_push_back(value2);
     * // guard 超出作用域时自动释放锁
     * ```
     */
    auto acquire_write_guard() const {
        return acquire_write_lock();
    }

    /**
     * @brief 获取读锁guard供外部使用
     * 
     * 仅在ReadWrite锁策略下有效
     * 使用示例：
     * ```cpp
     * auto guard = vec.acquire_read_guard();
     * auto val = vec.unsafe_at(0);
     * // guard 超出作用域时自动释放锁
     * ```
     */
#if TS_STL_SUPPORT_RW_LOCK
    auto acquire_read_guard() const {
        return acquire_read_lock();
    }
#endif

    /**
     * @brief 批量操作助手 - 在锁保护下执行lambda
     * 
     * 用于需要多个操作保持原子性的场景
     * 
     * 使用示例：
     * ```cpp
     * vec.with_write_lock([&](auto& v) {
     *     v.unsafe_push_back(1);
     *     v.unsafe_push_back(2);
     *     v.unsafe_push_back(3);
     * });
     * ```
     */
    template <typename Func>
    void with_write_lock(Func func) const {
        auto guard = acquire_write_lock();
        func(*const_cast<vector*>(this));
    }

    /**
     * @brief 批量读操作助手 - 在读锁保护下执行lambda
     * 
     * 仅在ReadWrite锁策略下有效
     * 
     * 使用示例：
     * ```cpp
     * vec.with_read_lock([&](const auto& v) {
     *     for (const auto& item : v.unsafe_ref()) {
     *         std::cout << item << std::endl;
     *     }
     * });
     * ```
     */
#if TS_STL_SUPPORT_RW_LOCK
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(*this);
    }
#endif
};

// ==================== LockFree 特化版本（零开销） ====================

/**
 * @brief Vector 的 LockFree 特化版本 - 为极限性能优化
 * 
 * 这是 vector<T, LockPolicy::LockFree> 的特化实现。
 * 提供与 std::vector 相同的性能（零开销），因为：
 * - 不需要加锁逻辑
 * - 直接访问内部数据
 * - 完全内联，无函数调用开销
 * 
 * 注意：此版本不提供任何线程同步保证。
 * 用户需确保：
 * 1. 单线程访问，或
 * 2. 多线程只读访问（无并发写），或
 * 3. 外部提供同步机制
 */
template <typename T>
class vector<T, LockPolicy::LockFree> {
private:
    std::vector<T> data_;

public:
    // 类型定义
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;
    using difference_type = typename std::vector<T>::difference_type;

    // ==================== 构造函数 ====================
    
    vector() = default;

    explicit vector(size_type count) : data_(count) {}

    vector(size_type count, const_reference value)
        : data_(count, value) {}

    template <typename InputIt>
    vector(InputIt first, InputIt last) : data_(first, last) {}

    vector(const vector& other) = default;

    vector& operator=(const vector& other) = default;

    vector(vector&& other) noexcept = default;

    vector& operator=(vector&& other) noexcept = default;

    // ==================== 元素访问（零开销） ====================

    /**
     * @brief 快速随机访问 - 完全零开销
     */
    const_reference operator[](size_type pos) const noexcept {
        return data_[pos];
    }

    reference operator[](size_type pos) noexcept {
        return data_[pos];
    }

    /**
     * @brief 安全的随机访问 - 带边界检查
     */
    const_reference at(size_type pos) const {
        return data_.at(pos);
    }

    reference at(size_type pos) {
        return data_.at(pos);
    }

    const_reference front() const noexcept {
        return data_.front();
    }

    reference front() noexcept {
        return data_.front();
    }

    const_reference back() const noexcept {
        return data_.back();
    }

    reference back() noexcept {
        return data_.back();
    }

    // ==================== 容量管理（零开销） ====================

    size_type size() const noexcept {
        return data_.size();
    }

    size_type capacity() const noexcept {
        return data_.capacity();
    }

    bool empty() const noexcept {
        return data_.empty();
    }

    void reserve(size_type new_cap) {
        data_.reserve(new_cap);
    }

    void shrink_to_fit() {
        data_.shrink_to_fit();
    }

    void clear() noexcept {
        data_.clear();
    }

    // ==================== 修改操作（零开销） ====================

    void push_back(const_reference value) {
        data_.push_back(value);
    }

    void push_back(value_type&& value) {
        data_.push_back(std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        return data_.emplace_back(std::forward<Args>(args)...);
    }

    void pop_back() noexcept {
        data_.pop_back();
    }

    iterator insert(const_iterator pos, const_reference value) {
        return data_.insert(pos, value);
    }

    iterator insert(const_iterator pos, value_type&& value) {
        return data_.insert(pos, std::move(value));
    }

    iterator erase(const_iterator pos) {
        return data_.erase(pos);
    }

    iterator erase(const_iterator first, const_iterator last) {
        return data_.erase(first, last);
    }

    // ==================== 查询操作（零开销） ====================

    iterator find(const_reference value) {
        return std::find(data_.begin(), data_.end(), value);
    }

    const_iterator find(const_reference value) const {
        return std::find(data_.cbegin(), data_.cend(), value);
    }

    size_type count(const_reference value) const {
        return std::count(data_.cbegin(), data_.cend(), value);
    }

    // ==================== 迭代器（零开销） ====================

    iterator begin() noexcept {
        return data_.begin();
    }

    const_iterator begin() const noexcept {
        return data_.begin();
    }

    const_iterator cbegin() const noexcept {
        return data_.cbegin();
    }

    iterator end() noexcept {
        return data_.end();
    }

    const_iterator end() const noexcept {
        return data_.end();
    }

    const_iterator cend() const noexcept {
        return data_.cend();
    }

    // ==================== 转换操作 ====================

    // 隐式转换到 std::vector<T>
    operator std::vector<T>&() noexcept {
        return data_;
    }

    operator const std::vector<T>&() const noexcept {
        return data_;
    }

    // 显式获取内部向量
    std::vector<T>& get_unsafe() noexcept {
        return data_;
    }

    const std::vector<T>& get_unsafe() const noexcept {
        return data_;
    }
};

} // namespace ts_stl

#endif // TS_VECTOR_HPP
