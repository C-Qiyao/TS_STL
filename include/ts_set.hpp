#pragma once

#ifndef TS_SET_HPP
#define TS_SET_HPP

#include <set>

#include "ts_stl_base.hpp"

namespace ts_stl {

/**
 * @brief 线程安全的Set代理类
 * @tparam Key 键类型
 * @tparam Compare 比较器（默认使用std::less）
 * @tparam Policy 锁策略（默认使用互斥锁）
 */
template <typename Key, typename Compare = std::less<Key>, LockPolicy Policy = LockPolicy::Mutex>
class set : public container_mixin<set<Key, Compare, Policy>, Key, Policy> {
private:
    friend class container_mixin<set<Key, Compare, Policy>, Key, Policy>;

    std::set<Key, Compare> data_;

    using Base = container_mixin<set<Key, Compare, Policy>, Key, Policy>;
    using Base::acquire_write_lock;
    using Base::acquire_read_lock;

public:
    using Container = std::set<Key, Compare>;
    using value_type = Key;
    using size_type = typename std::set<Key, Compare>::size_type;
    using iterator = typename std::set<Key, Compare>::iterator;
    using const_iterator = typename std::set<Key, Compare>::const_iterator;

    // ==================== 构造函数 ====================

    set() : Base() {}

    explicit set(const Compare& comp) : Base() {
        data_ = std::set<Key, Compare>(comp);
    }

    template <typename InputIt>
    set(InputIt first, InputIt last) : Base() {
        data_.insert(first, last);
    }

    set(const set& other) : Base() {
        auto guard = acquire_write_lock();
        data_ = other.data_;
    }

    set& operator=(const set& other) {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = other.data_;
        }
        return *this;
    }

    set(set&& other) noexcept : Base() {
        auto guard = acquire_write_lock();
        data_ = std::move(other.data_);
    }

    set& operator=(set&& other) noexcept {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = std::move(other.data_);
        }
        return *this;
    }

    // ==================== 容量管理 ====================

    size_type size() const {
        auto guard = acquire_read_lock();
        return data_.size();
    }

    bool empty() const {
        auto guard = acquire_read_lock();
        return data_.empty();
    }

    void clear() {
        auto guard = acquire_write_lock();
        data_.clear();
    }

    // ==================== 查找操作 ====================

    bool contains(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.find(key) != data_.end();
    }

    size_type count(const Key& key) const {
        auto guard = acquire_read_lock();
        return data_.count(key);
    }

    // ==================== 修改操作 ====================

    std::pair<iterator, bool> insert(const Key& key) {
        auto guard = acquire_write_lock();
        return data_.insert(key);
    }

    std::pair<iterator, bool> insert(Key&& key) {
        auto guard = acquire_write_lock();
        return data_.insert(std::move(key));
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        auto guard = acquire_write_lock();
        return data_.emplace(std::forward<Args>(args)...);
    }

    size_type erase(const Key& key) {
        auto guard = acquire_write_lock();
        return data_.erase(key);
    }

    void erase(const_iterator pos) {
        auto guard = acquire_write_lock();
        data_.erase(pos);
    }

    void erase(const_iterator first, const_iterator last) {
        auto guard = acquire_write_lock();
        data_.erase(first, last);
    }

    // ==================== STL兼容性 ====================

    operator const std::set<Key, Compare>&() const {
        return data_;
    }

    std::set<Key, Compare> copy() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    const std::set<Key, Compare>& ref() const {
        return data_;
    }

    // ==================== 迭代和查询 ====================

    template <typename Func>
    void for_each(Func func) const {
        auto guard = acquire_read_lock();
        for (const auto& item : data_) {
            func(item);
        }
    }

    template <typename Predicate>
    const_iterator find_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::find_if(data_.begin(), data_.end(), pred);
    }

    template <typename Predicate>
    size_type count_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::count_if(data_.begin(), data_.end(), pred);
    }

    // ==================== 线程不安全接口 ====================

    std::set<Key, Compare>& unsafe_ref() {
        return data_;
    }

    const std::set<Key, Compare>& unsafe_ref() const {
        return data_;
    }

    size_type unsafe_size() const {
        return data_.size();
    }

    bool unsafe_empty() const {
        return data_.empty();
    }

    void unsafe_clear() {
        data_.clear();
    }

    std::pair<iterator, bool> unsafe_insert(const Key& key) {
        return data_.insert(key);
    }

    size_type unsafe_erase(const Key& key) {
        return data_.erase(key);
    }

    // ==================== 手动锁控制接口 ====================

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
        func(*const_cast<set*>(this));
    }

#if TS_STL_SUPPORT_RW_LOCK
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(*this);
    }
#endif
};

// ==================== Set 的 LockFree 特化版本 ====================

template <typename Key, typename Compare>
class set<Key, Compare, LockPolicy::LockFree> {
private:
    std::set<Key, Compare> data_;

public:
    using value_type = Key;
    using size_type = typename std::set<Key, Compare>::size_type;
    using iterator = typename std::set<Key, Compare>::iterator;
    using const_iterator = typename std::set<Key, Compare>::const_iterator;

    set() = default;

    explicit set(const Compare& comp) : data_(comp) {}

    template <typename InputIt>
    set(InputIt first, InputIt last) : data_(first, last) {}

    set(const set& other) = default;
    set& operator=(const set& other) = default;
    set(set&& other) noexcept = default;
    set& operator=(set&& other) noexcept = default;

    size_type size() const noexcept {
        return data_.size();
    }

    bool empty() const noexcept {
        return data_.empty();
    }

    void clear() noexcept {
        data_.clear();
    }

    bool contains(const Key& key) const {
        return data_.find(key) != data_.end();
    }

    size_type count(const Key& key) const {
        return data_.count(key);
    }

    std::pair<iterator, bool> insert(const Key& key) {
        return data_.insert(key);
    }

    std::pair<iterator, bool> insert(Key&& key) {
        return data_.insert(std::move(key));
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return data_.emplace(std::forward<Args>(args)...);
    }

    size_type erase(const Key& key) {
        return data_.erase(key);
    }

    operator std::set<Key, Compare>&() noexcept {
        return data_;
    }

    operator const std::set<Key, Compare>&() const noexcept {
        return data_;
    }

    std::set<Key, Compare>& get_unsafe() noexcept {
        return data_;
    }

    const std::set<Key, Compare>& get_unsafe() const noexcept {
        return data_;
    }
};

} // namespace ts_stl

#endif // TS_SET_HPP
