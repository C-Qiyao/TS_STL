#pragma once

#ifndef TS_UNORDERED_SET_HPP
#define TS_UNORDERED_SET_HPP

#include <unordered_set>

#include "ts_stl_base.hpp"

namespace ts_stl {

/**
 * @brief 线程安全的Unordered Set代理类
 * @tparam Key 键类型
 * @tparam Hash 哈希函数（默认使用std::hash）
 * @tparam KeyEqual 键相等比较器（默认使用std::equal_to）
 * @tparam Policy 锁策略（默认使用互斥锁）
 */
template <typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, LockPolicy Policy = LockPolicy::Mutex>
class unordered_set : public container_mixin<unordered_set<Key, Hash, KeyEqual, Policy>, Key, Policy> {
private:
    friend class container_mixin<unordered_set<Key, Hash, KeyEqual, Policy>, Key, Policy>;

    std::unordered_set<Key, Hash, KeyEqual> data_;

    using Base = container_mixin<unordered_set<Key, Hash, KeyEqual, Policy>, Key, Policy>;
    using Base::acquire_write_lock;
    using Base::acquire_read_lock;

public:
    using Container = std::unordered_set<Key, Hash, KeyEqual>;
    using value_type = Key;
    using size_type = typename std::unordered_set<Key, Hash, KeyEqual>::size_type;
    using iterator = typename std::unordered_set<Key, Hash, KeyEqual>::iterator;
    using const_iterator = typename std::unordered_set<Key, Hash, KeyEqual>::const_iterator;

    // ==================== 构造函数 ====================

    unordered_set() : Base() {}

    explicit unordered_set(size_type bucket_count) : Base() {
        data_ = std::unordered_set<Key, Hash, KeyEqual>(bucket_count);
    }

    unordered_set(size_type bucket_count, const Hash& hash) : Base() {
        data_ = std::unordered_set<Key, Hash, KeyEqual>(bucket_count, hash);
    }

    unordered_set(size_type bucket_count, const Hash& hash, const KeyEqual& equal) : Base() {
        data_ = std::unordered_set<Key, Hash, KeyEqual>(bucket_count, hash, equal);
    }

    template <typename InputIt>
    unordered_set(InputIt first, InputIt last) : Base() {
        data_.insert(first, last);
    }

    unordered_set(const unordered_set& other) : Base() {
        auto guard = acquire_write_lock();
        data_ = other.data_;
    }

    unordered_set& operator=(const unordered_set& other) {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = other.data_;
        }
        return *this;
    }

    unordered_set(unordered_set&& other) noexcept : Base() {
        auto guard = acquire_write_lock();
        data_ = std::move(other.data_);
    }

    unordered_set& operator=(unordered_set&& other) noexcept {
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

    size_type bucket_count() const {
        auto guard = acquire_read_lock();
        return data_.bucket_count();
    }

    float load_factor() const {
        auto guard = acquire_read_lock();
        return data_.load_factor();
    }

    float max_load_factor() const {
        auto guard = acquire_read_lock();
        return data_.max_load_factor();
    }

    void max_load_factor(float ml) {
        auto guard = acquire_write_lock();
        data_.max_load_factor(ml);
    }

    void reserve(size_type n) {
        auto guard = acquire_write_lock();
        data_.reserve(n);
    }

    void rehash(size_type n) {
        auto guard = acquire_write_lock();
        data_.rehash(n);
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

    operator const std::unordered_set<Key, Hash, KeyEqual>&() const {
        return data_;
    }

    std::unordered_set<Key, Hash, KeyEqual> copy() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    const std::unordered_set<Key, Hash, KeyEqual>& ref() const {
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

    std::unordered_set<Key, Hash, KeyEqual>& unsafe_ref() {
        return data_;
    }

    const std::unordered_set<Key, Hash, KeyEqual>& unsafe_ref() const {
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
        func(*const_cast<unordered_set*>(this));
    }

#if TS_STL_SUPPORT_RW_LOCK
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(*this);
    }
#endif
};

// ==================== Unordered Set 的 LockFree 特化版本 ====================

template <typename Key, typename Hash, typename KeyEqual>
class unordered_set<Key, Hash, KeyEqual, LockPolicy::LockFree> {
private:
    std::unordered_set<Key, Hash, KeyEqual> data_;

public:
    using value_type = Key;
    using size_type = typename std::unordered_set<Key, Hash, KeyEqual>::size_type;
    using iterator = typename std::unordered_set<Key, Hash, KeyEqual>::iterator;
    using const_iterator = typename std::unordered_set<Key, Hash, KeyEqual>::const_iterator;

    unordered_set() = default;

    explicit unordered_set(size_type bucket_count) : data_(bucket_count) {}

    unordered_set(size_type bucket_count, const Hash& hash) : data_(bucket_count, hash) {}

    unordered_set(size_type bucket_count, const Hash& hash, const KeyEqual& equal) 
        : data_(bucket_count, hash, equal) {}

    template <typename InputIt>
    unordered_set(InputIt first, InputIt last) : data_(first, last) {}

    unordered_set(const unordered_set& other) = default;
    unordered_set& operator=(const unordered_set& other) = default;
    unordered_set(unordered_set&& other) noexcept = default;
    unordered_set& operator=(unordered_set&& other) noexcept = default;

    size_type size() const noexcept {
        return data_.size();
    }

    bool empty() const noexcept {
        return data_.empty();
    }

    void clear() noexcept {
        data_.clear();
    }

    size_type bucket_count() const noexcept {
        return data_.bucket_count();
    }

    float load_factor() const noexcept {
        return data_.load_factor();
    }

    float max_load_factor() const noexcept {
        return data_.max_load_factor();
    }

    void max_load_factor(float ml) noexcept {
        data_.max_load_factor(ml);
    }

    void reserve(size_type n) noexcept {
        data_.reserve(n);
    }

    void rehash(size_type n) noexcept {
        data_.rehash(n);
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

    operator std::unordered_set<Key, Hash, KeyEqual>&() noexcept {
        return data_;
    }

    operator const std::unordered_set<Key, Hash, KeyEqual>&() const noexcept {
        return data_;
    }

    std::unordered_set<Key, Hash, KeyEqual>& get_unsafe() noexcept {
        return data_;
    }

    const std::unordered_set<Key, Hash, KeyEqual>& get_unsafe() const noexcept {
        return data_;
    }
};

} // namespace ts_stl

#endif // TS_UNORDERED_SET_HPP
