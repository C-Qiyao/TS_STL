#pragma once

#ifndef TS_MAP_HPP
#define TS_MAP_HPP

#include <map>

#include "ts_stl_base.hpp"

namespace ts_stl {

/**
 * @brief 线程安全的Map代理类
 * @tparam Key 键类型
 * @tparam T 值类型
 * @tparam Compare 比较器（默认使用std::less）
 * @tparam Policy 锁策略（默认使用互斥锁）
 */
template <typename Key, typename T, typename Compare = std::less<Key>, LockPolicy Policy = LockPolicy::Mutex>
class map : public container_mixin<map<Key, T, Compare, Policy>, std::pair<const Key, T>, Policy> {
private:
    friend class container_mixin<map<Key, T, Compare, Policy>, std::pair<const Key, T>, Policy>;

    std::map<Key, T, Compare> data_;

    using Base = container_mixin<map<Key, T, Compare, Policy>, std::pair<const Key, T>, Policy>;
    using Base::acquire_write_lock;
    using Base::acquire_read_lock;

public:
    // 为基类提供必要的类型信息
    using Container = std::map<Key, T, Compare>;
    
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = typename std::map<Key, T, Compare>::size_type;
    using iterator = typename std::map<Key, T, Compare>::iterator;
    using const_iterator = typename std::map<Key, T, Compare>::const_iterator;
    using reverse_iterator = typename std::map<Key, T, Compare>::reverse_iterator;
    using const_reverse_iterator = typename std::map<Key, T, Compare>::const_reverse_iterator;

    // ==================== 构造函数 ====================

    map() : Base() {}

    explicit map(const Compare& comp) : Base() {
        data_ = std::map<Key, T, Compare>(comp);
    }

    template <typename InputIt>
    map(InputIt first, InputIt last) : Base() {
        data_.insert(first, last);
    }

    map(const map& other) : Base() {
        auto guard = acquire_write_lock();
        data_ = other.data_;
    }

    map& operator=(const map& other) {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = other.data_;
        }
        return *this;
    }

    map(map&& other) noexcept : Base() {
        auto guard = acquire_write_lock();
        data_ = std::move(other.data_);
    }

    map& operator=(map&& other) noexcept {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = std::move(other.data_);
        }
        return *this;
    }

    // ==================== 元素访问 ====================

    /**
     * @brief 获取指定键对应的值（如果键不存在则插入默认值）
     */
    T& operator[](const Key& key) {
        auto guard = acquire_write_lock();
        return data_[key];
    }

    /**
     * @brief 获取指定键对应的值（const版本）
     */
    T at(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.at(key);
    }

    /**
     * @brief 获取指定键对应的值（非const版本）
     */
    T& at(const Key& key) {
        auto guard = acquire_write_lock();
        return data_.at(key);
    }

    /**
     * @brief 设置指定键的值
     */
    void set(const Key& key, const T& value) {
        auto guard = acquire_write_lock();
        data_[key] = value;
    }

    /**
     * @brief 获取指定键的值，如果不存在返回默认值
     */
    T get(const Key& key, const T& default_value = T()) const {
        auto guard = acquire_read_lock();
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return default_value;
    }

    // ==================== 容量管理 ====================

    /**
     * @brief 获取map的大小
     */
    size_type size() const {
        auto guard = acquire_read_lock();
        return data_.size();
    }

    /**
     * @brief 检查map是否为空
     */
    bool empty() const {
        auto guard = acquire_read_lock();
        return data_.empty();
    }

    /**
     * @brief 清空map
     */
    void clear() {
        auto guard = acquire_write_lock();
        data_.clear();
    }

    // ==================== 查找操作 ====================

    /**
     * @brief 查找指定键
     */
    bool contains(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.find(key) != data_.end();
    }

    /**
     * @brief 统计指定键的个数
     */
    size_type count(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.count(key);
    }

    /**
     * @brief 查找第一个不小于指定键的元素
     */
    bool lower_bound(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.lower_bound(key) != data_.end();
    }

    /**
     * @brief 查找第一个大于指定键的元素
     */
    bool upper_bound(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.upper_bound(key) != data_.end();
    }

    // ==================== 修改操作 ====================

    /**
     * @brief 插入键值对
     */
    std::pair<bool, size_type> insert(const Key& key, const T& value) {
        auto guard = acquire_write_lock();
        auto result = data_.insert({key, value});
        return {result.second, std::distance(data_.begin(), result.first)};
    }

    /**
     * @brief 使用移动语义插入键值对
     */
    std::pair<bool, size_type> insert(const Key& key, T&& value) {
        auto guard = acquire_write_lock();
        auto result = data_.insert({key, std::move(value)});
        return {result.second, std::distance(data_.begin(), result.first)};
    }

    /**
     * @brief 原地构造并插入
     */
    template <typename... Args>
    std::pair<bool, size_type> emplace(const Key& key, Args&&... args) {
        auto guard = acquire_write_lock();
        auto result = data_.emplace(key, T(std::forward<Args>(args)...));
        return {result.second, std::distance(data_.begin(), result.first)};
    }

    /**
     * @brief 移除指定键
     */
    size_type erase(const Key& key) {
        auto guard = acquire_write_lock();
        return data_.erase(key);
    }

    /**
     * @brief 删除指定位置的元素
     */
    void erase(const_iterator pos) {
        auto guard = acquire_write_lock();
        data_.erase(pos);
    }

    /**
     * @brief 删除指定范围的元素
     */
    void erase(const_iterator first, const_iterator last) {
        auto guard = acquire_write_lock();
        data_.erase(first, last);
    }

    // ==================== STL兼容性 ====================

    /**
     * @brief 隐式转换到std::map
     */
    operator const std::map<Key, T, Compare>&() const {
        return data_;
    }

    /**
     * @brief 获取内部map的拷贝
     */
    std::map<Key, T, Compare> copy() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    /**
     * @brief 获取内部map的引用
     */
    const std::map<Key, T, Compare>& ref() const {
        return data_;
    }

    // ==================== 迭代和查询 ====================

    /**
     * @brief 对每个元素执行操作
     */
    template <typename Func>
    void for_each(Func func) const {
        auto guard = acquire_read_lock();
        for (const auto& pair : data_) {
            func(pair.first, pair.second);
        }
    }

    /**
     * @brief 条件查找
     */
    template <typename Predicate>
    const_iterator find_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::find_if(data_.begin(), data_.end(), 
                          [&pred](const auto& p) { return pred(p.first, p.second); });
    }

    /**
     * @brief 统计满足条件的元素个数
     */
    template <typename Predicate>
    size_type count_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::count_if(data_.begin(), data_.end(),
                            [&pred](const auto& p) { return pred(p.first, p.second); });
    }

    // ==================== 比较操作 ====================

    bool operator==(const map& other) const {
        auto guard = acquire_read_lock();
        return data_ == other.data_;
    }

    bool operator!=(const map& other) const {
        return !(*this == other);
    }

    // ==================== 线程不安全接口 ====================

    /**
     * @brief 获取内部map的非const引用（线程不安全）
     */
    std::map<Key, T, Compare>& unsafe_ref() {
        return data_;
    }

    const std::map<Key, T, Compare>& unsafe_ref() const {
        return data_;
    }

    /**
     * @brief 线程不安全的size获取
     */
    size_type unsafe_size() const {
        return data_.size();
    }

    /**
     * @brief 线程不安全的empty检查
     */
    bool unsafe_empty() const {
        return data_.empty();
    }

    /**
     * @brief 线程不安全的clear
     */
    void unsafe_clear() {
        data_.clear();
    }

    /**
     * @brief 线程不安全的insert
     */
    void unsafe_insert(const Key& key, const T& value) {
        data_[key] = value;
    }

    void unsafe_insert(const Key& key, T&& value) {
        data_[key] = std::move(value);
    }

    /**
     * @brief 线程不安全的erase
     */
    size_type unsafe_erase(const Key& key) {
        return data_.erase(key);
    }

    /**
     * @brief 线程不安全的at访问
     */
    T& unsafe_at(const Key& key) {
        return data_[key];
    }

    const T& unsafe_at(const Key& key) const {
        return data_.at(key);
    }

    // ==================== 手动锁控制接口 ====================

    /**
     * @brief 获取写锁guard供外部使用
     */
    auto acquire_write_guard() const {
        return acquire_write_lock();
    }

#if TS_STL_SUPPORT_RW_LOCK
    /**
     * @brief 获取读锁guard供外部使用
     */
    auto acquire_read_guard() const {
        return acquire_read_lock();
    }
#endif

    /**
     * @brief 批量操作助手 - 在锁保护下执行lambda
     */
    template <typename Func>
    void with_write_lock(Func func) const {
        auto guard = acquire_write_lock();
        func(*const_cast<map*>(this));
    }

#if TS_STL_SUPPORT_RW_LOCK
    /**
     * @brief 批量读操作助手 - 在读锁保护下执行lambda
     */
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(*this);
    }
#endif
};

// ==================== Map 的 LockFree 特化版本（零开销） ====================

/**
 * @brief Map 的 LockFree 特化版本 - 为极限性能优化
 */
template <typename Key, typename T, typename Compare>
class map<Key, T, Compare, LockPolicy::LockFree> {
private:
    std::map<Key, T, Compare> data_;

public:
    // 类型定义
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = typename std::map<Key, T, Compare>::size_type;
    using iterator = typename std::map<Key, T, Compare>::iterator;
    using const_iterator = typename std::map<Key, T, Compare>::const_iterator;

    // ==================== 构造函数 ====================

    map() = default;

    explicit map(const Compare& comp) : data_(comp) {}

    template <typename InputIt>
    map(InputIt first, InputIt last) : data_(first, last) {}

    map(const map& other) = default;

    map& operator=(const map& other) = default;

    map(map&& other) noexcept = default;

    map& operator=(map&& other) noexcept = default;

    // ==================== 元素访问（零开销） ====================

    T& operator[](const Key& key) noexcept {
        return data_[key];
    }

    T at(const Key& key) const {
        return data_.at(key);
    }

    T& at(const Key& key) {
        return data_.at(key);
    }

    T get(const Key& key, const T& default_value = T()) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return default_value;
    }

    // ==================== 容量管理（零开销） ====================

    size_type size() const noexcept {
        return data_.size();
    }

    bool empty() const noexcept {
        return data_.empty();
    }

    void clear() noexcept {
        data_.clear();
    }

    // ==================== 查找操作（零开销） ====================

    bool contains(const Key& key) const {
        return data_.find(key) != data_.end();
    }

    size_type count(const Key& key) const {
        return data_.count(key);
    }

    // ==================== 修改操作（零开销） ====================

    std::pair<bool, size_type> insert(const Key& key, const T& value) {
        auto result = data_.insert({key, value});
        return {result.second, std::distance(data_.begin(), result.first)};
    }

    std::pair<bool, size_type> insert(const Key& key, T&& value) {
        auto result = data_.insert({key, std::move(value)});
        return {result.second, std::distance(data_.begin(), result.first)};
    }

    template <typename... Args>
    std::pair<bool, size_type> emplace(const Key& key, Args&&... args) {
        auto result = data_.emplace(key, T(std::forward<Args>(args)...));
        return {result.second, std::distance(data_.begin(), result.first)};
    }

    size_type erase(const Key& key) {
        return data_.erase(key);
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

    operator std::map<Key, T, Compare>&() noexcept {
        return data_;
    }

    operator const std::map<Key, T, Compare>&() const noexcept {
        return data_;
    }

    std::map<Key, T, Compare>& get_unsafe() noexcept {
        return data_;
    }

    const std::map<Key, T, Compare>& get_unsafe() const noexcept {
        return data_;
    }
};

} // namespace ts_stl

#endif // TS_MAP_HPP
