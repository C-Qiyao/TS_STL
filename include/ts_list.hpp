#pragma once

#ifndef TS_LIST_HPP
#define TS_LIST_HPP

#include <list>
#include <stdexcept>
#include <algorithm>
#include "ts_stl_base.hpp"

namespace ts_stl {

/**
 * @brief 线程安全的List代理类
 * @tparam T 元素类型
 * @tparam Policy 锁策略（默认使用互斥锁）
 */
template <typename T, LockPolicy Policy = LockPolicy::Mutex>
class list : public container_mixin<list<T, Policy>, T, Policy> {
private:
    friend class container_mixin<list<T, Policy>, T, Policy>;

    std::list<T> data_;

    using Base = container_mixin<list<T, Policy>, T, Policy>;
    using Base::acquire_write_lock;
    using Base::acquire_read_lock;

public:
    // 为基类提供必要的类型信息
    using Container = std::list<T>;
    
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::list<T>::iterator;
    using const_iterator = typename std::list<T>::const_iterator;
    using size_type = typename std::list<T>::size_type;
    using difference_type = typename std::list<T>::difference_type;

    list() : Base() {}

    explicit list(size_type count) 
        : Base() {
        data_.resize(count);
    }

    list(size_type count, const_reference value)
        : Base() {
        data_.assign(count, value);
    }

    template <typename InputIt>
    list(InputIt first, InputIt last)
        : Base() {
        data_.assign(first, last);
    }

    list(const list& other) : Base() {
        auto guard = acquire_write_lock();
        data_ = other.data_;
    }

    list& operator=(const list& other) {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = other.data_;
        }
        return *this;
    }

    list(list&& other) noexcept : Base() {
        auto guard = acquire_write_lock();
        data_ = std::move(other.data_);
    }

    list& operator=(list&& other) noexcept {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = std::move(other.data_);
        }
        return *this;
    }

    // ==================== 元素访问 ====================

    const_reference front() const {
        auto guard = acquire_read_lock();
        if (data_.empty()) {
            throw std::out_of_range("List is empty");
        }
        return data_.front();
    }

    const_reference back() const {
        auto guard = acquire_read_lock();
        if (data_.empty()) {
            throw std::out_of_range("List is empty");
        }
        return data_.back();
    }

    void set_front(const_reference value) {
        auto guard = acquire_write_lock();
        if (data_.empty()) {
            throw std::out_of_range("List is empty");
        }
        data_.front() = value;
    }

    void set_back(const_reference value) {
        auto guard = acquire_write_lock();
        if (data_.empty()) {
            throw std::out_of_range("List is empty");
        }
        data_.back() = value;
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

    void resize(size_type count) {
        auto guard = acquire_write_lock();
        data_.resize(count);
    }

    void resize(size_type count, const_reference value) {
        auto guard = acquire_write_lock();
        data_.resize(count, value);
    }

    void clear() {
        auto guard = acquire_write_lock();
        data_.clear();
    }

    // ==================== 修改操作 ====================

    void push_back(const_reference value) {
        auto guard = acquire_write_lock();
        data_.push_back(value);
    }

    void push_back(T&& value) {
        auto guard = acquire_write_lock();
        data_.push_back(std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        auto guard = acquire_write_lock();
        return data_.emplace_back(std::forward<Args>(args)...);
    }

    void pop_back() {
        auto guard = acquire_write_lock();
        if (data_.empty()) {
            throw std::out_of_range("Cannot pop from empty list");
        }
        data_.pop_back();
    }

    void push_front(const_reference value) {
        auto guard = acquire_write_lock();
        data_.push_front(value);
    }

    void push_front(T&& value) {
        auto guard = acquire_write_lock();
        data_.push_front(std::move(value));
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        auto guard = acquire_write_lock();
        return data_.emplace_front(std::forward<Args>(args)...);
    }

    void pop_front() {
        auto guard = acquire_write_lock();
        if (data_.empty()) {
            throw std::out_of_range("Cannot pop from empty list");
        }
        data_.pop_front();
    }

    iterator insert(const_iterator pos, const_reference value) {
        auto guard = acquire_write_lock();
        return data_.insert(pos, value);
    }

    iterator erase(const_iterator pos) {
        auto guard = acquire_write_lock();
        return data_.erase(pos);
    }

    iterator erase(const_iterator first, const_iterator last) {
        auto guard = acquire_write_lock();
        return data_.erase(first, last);
    }

    size_type remove(const T& value) {
        auto guard = acquire_write_lock();
        size_type count = 0;
        auto it = data_.begin();
        while (it != data_.end()) {
            if (*it == value) {
                it = data_.erase(it);
                ++count;
            } else {
                ++it;
            }
        }
        return count;
    }

    template <typename Predicate>
    size_type remove_if(Predicate pred) {
        auto guard = acquire_write_lock();
        size_type count = 0;
        auto it = data_.begin();
        while (it != data_.end()) {
            if (pred(*it)) {
                it = data_.erase(it);
                ++count;
            } else {
                ++it;
            }
        }
        return count;
    }

    void reverse() {
        auto guard = acquire_write_lock();
        data_.reverse();
    }

    template <typename Compare = std::less<T>>
    void sort(Compare comp = Compare()) {
        auto guard = acquire_write_lock();
        data_.sort(comp);
    }

    // ==================== STL兼容性 ====================

    operator const std::list<T>&() const {
        return data_;
    }

    std::list<T> copy() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    const std::list<T>& ref() const {
        return data_;
    }

    // ==================== 迭代和查询 ====================

    template <typename Func>
    void for_each(Func func) const {
        auto guard = acquire_read_lock();
        std::for_each(data_.begin(), data_.end(), func);
    }

    template <typename Predicate>
    const_iterator find_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::find_if(data_.begin(), data_.end(), pred);
    }

    bool contains(const T& value) const {
        auto guard = acquire_read_lock();
        return std::find(data_.begin(), data_.end(), value) != data_.end();
    }

    size_type count(const T& value) const {
        auto guard = acquire_read_lock();
        return static_cast<size_type>(std::count(data_.begin(), data_.end(), value));
    }

    // ==================== 线程不安全接口 ====================

    std::list<T>& unsafe_ref() {
        return data_;
    }

    const std::list<T>& unsafe_ref() const {
        return data_;
    }

    size_type unsafe_size() const {
        return data_.size();
    }

    bool unsafe_empty() const {
        return data_.empty();
    }

    void unsafe_push_back(const_reference value) {
        data_.push_back(value);
    }

    void unsafe_push_back(T&& value) {
        data_.push_back(std::move(value));
    }

    void unsafe_pop_back() {
        if (!data_.empty()) {
            data_.pop_back();
        }
    }

    void unsafe_push_front(const_reference value) {
        data_.push_front(value);
    }

    void unsafe_push_front(T&& value) {
        data_.push_front(std::move(value));
    }

    void unsafe_pop_front() {
        if (!data_.empty()) {
            data_.pop_front();
        }
    }

    void unsafe_clear() {
        data_.clear();
    }

    reference unsafe_front() {
        return data_.front();
    }

    const_reference unsafe_front() const {
        return data_.front();
    }

    reference unsafe_back() {
        return data_.back();
    }

    const_reference unsafe_back() const {
        return data_.back();
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
        func(*const_cast<list*>(this));
    }

#if TS_STL_SUPPORT_RW_LOCK
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(*this);
    }
#endif
};

// ==================== List 的 LockFree 特化版本（零开销） ====================

/**
 * @brief List 的 LockFree 特化版本 - 为极限性能优化
 */
template <typename T>
class list<T, LockPolicy::LockFree> {
private:
    std::list<T> data_;

public:
    // 类型定义
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::list<T>::iterator;
    using const_iterator = typename std::list<T>::const_iterator;
    using size_type = typename std::list<T>::size_type;
    using difference_type = typename std::list<T>::difference_type;

    // ==================== 构造函数 ====================

    list() = default;

    explicit list(size_type count) : data_(count) {}

    list(size_type count, const_reference value)
        : data_(count, value) {}

    template <typename InputIt>
    list(InputIt first, InputIt last) : data_(first, last) {}

    list(const list& other) = default;

    list& operator=(const list& other) = default;

    list(list&& other) noexcept = default;

    list& operator=(list&& other) noexcept = default;

    // ==================== 元素访问（零开销） ====================

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

    bool empty() const noexcept {
        return data_.empty();
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

    void push_front(const_reference value) {
        data_.push_front(value);
    }

    void push_front(value_type&& value) {
        data_.push_front(std::move(value));
    }

    void pop_front() noexcept {
        data_.pop_front();
    }

    iterator insert(const_iterator pos, const_reference value) {
        return data_.insert(pos, value);
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

    operator std::list<T>&() noexcept {
        return data_;
    }

    operator const std::list<T>&() const noexcept {
        return data_;
    }

    std::list<T>& get_unsafe() noexcept {
        return data_;
    }

    const std::list<T>& get_unsafe() const noexcept {
        return data_;
    }
};

} // namespace ts_stl

#endif // TS_LIST_HPP
