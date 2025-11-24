#pragma once

#ifndef TS_DEQUE_HPP
#define TS_DEQUE_HPP

#include <deque>

#include "ts_stl_base.hpp"

namespace ts_stl {

/**
 * @brief 线程安全的Deque代理类
 * @tparam T 元素类型
 * @tparam Policy 锁策略（默认使用互斥锁）
 */
template <typename T, LockPolicy Policy = LockPolicy::Mutex>
class deque : public container_mixin<deque<T, Policy>, T, Policy> {
private:
    friend class container_mixin<deque<T, Policy>, T, Policy>;

    std::deque<T> data_;

    using Base = container_mixin<deque<T, Policy>, T, Policy>;
    using Base::acquire_write_lock;
    using Base::acquire_read_lock;

public:
    using Container = std::deque<T>;
    using value_type = T;
    using size_type = typename std::deque<T>::size_type;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::deque<T>::iterator;
    using const_iterator = typename std::deque<T>::const_iterator;

    // ==================== 构造函数 ====================

    deque() : Base() {}

    explicit deque(size_type count) : Base() {
        data_ = std::deque<T>(count);
    }

    deque(size_type count, const T& value) : Base() {
        data_ = std::deque<T>(count, value);
    }

    template <typename InputIt>
    deque(InputIt first, InputIt last) : Base() {
        data_.insert(data_.end(), first, last);
    }

    deque(const deque& other) : Base() {
        auto guard = acquire_write_lock();
        data_ = other.data_;
    }

    deque& operator=(const deque& other) {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = other.data_;
        }
        return *this;
    }

    deque(deque&& other) noexcept : Base() {
        auto guard = acquire_write_lock();
        data_ = std::move(other.data_);
    }

    deque& operator=(deque&& other) noexcept {
        if (this != &other) {
            auto guard = acquire_write_lock();
            data_ = std::move(other.data_);
        }
        return *this;
    }

    // ==================== 元素访问 ====================

    reference operator[](size_type index) {
        auto guard = acquire_write_lock();
        return data_[index];
    }

    const_reference operator[](size_type index) const {
        auto guard = acquire_read_lock();
        return data_[index];
    }

    reference at(size_type index) {
        auto guard = acquire_write_lock();
        return data_.at(index);
    }

    const_reference at(size_type index) const {
        auto guard = acquire_read_lock();
        return data_.at(index);
    }

    T get(size_type index, const T& default_value = T()) const {
        auto guard = acquire_read_lock();
        if (index < data_.size()) {
            return data_[index];
        }
        return default_value;
    }

    void set(size_type index, const T& value) {
        auto guard = acquire_write_lock();
        if (index < data_.size()) {
            data_[index] = value;
        }
    }

    reference front() {
        auto guard = acquire_write_lock();
        return data_.front();
    }

    const_reference front() const {
        auto guard = acquire_read_lock();
        return data_.front();
    }

    reference back() {
        auto guard = acquire_write_lock();
        return data_.back();
    }

    const_reference back() const {
        auto guard = acquire_read_lock();
        return data_.back();
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

    void resize(size_type count, const T& value) {
        auto guard = acquire_write_lock();
        data_.resize(count, value);
    }

    void clear() {
        auto guard = acquire_write_lock();
        data_.clear();
    }

    void shrink_to_fit() {
        auto guard = acquire_write_lock();
        data_.shrink_to_fit();
    }

    // ==================== 修改操作 ====================

    void push_back(const T& value) {
        auto guard = acquire_write_lock();
        data_.push_back(value);
    }

    void push_back(T&& value) {
        auto guard = acquire_write_lock();
        data_.push_back(std::move(value));
    }

    void pop_back() {
        auto guard = acquire_write_lock();
        if (!data_.empty()) {
            data_.pop_back();
        }
    }

    void push_front(const T& value) {
        auto guard = acquire_write_lock();
        data_.push_front(value);
    }

    void push_front(T&& value) {
        auto guard = acquire_write_lock();
        data_.push_front(std::move(value));
    }

    void pop_front() {
        auto guard = acquire_write_lock();
        if (!data_.empty()) {
            data_.pop_front();
        }
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        auto guard = acquire_write_lock();
        data_.emplace_back(std::forward<Args>(args)...);
        return data_.back();
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        auto guard = acquire_write_lock();
        data_.emplace_front(std::forward<Args>(args)...);
        return data_.front();
    }

    // ==================== STL兼容性 ====================

    operator const std::deque<T>&() const {
        return data_;
    }

    std::deque<T> copy() const {
        auto guard = acquire_read_lock();
        return data_;
    }

    const std::deque<T>& ref() const {
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

    bool contains(const T& value) const {
        auto guard = acquire_read_lock();
        return std::find(data_.begin(), data_.end(), value) != data_.end();
    }

    template <typename Predicate>
    size_type count_if(Predicate pred) const {
        auto guard = acquire_read_lock();
        return std::count_if(data_.begin(), data_.end(), pred);
    }

    // ==================== 线程不安全接口 ====================

    std::deque<T>& unsafe_ref() {
        return data_;
    }

    const std::deque<T>& unsafe_ref() const {
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

    void unsafe_push_back(const T& value) {
        data_.push_back(value);
    }

    void unsafe_push_front(const T& value) {
        data_.push_front(value);
    }

    void unsafe_pop_back() {
        if (!data_.empty()) {
            data_.pop_back();
        }
    }

    void unsafe_pop_front() {
        if (!data_.empty()) {
            data_.pop_front();
        }
    }

    T& unsafe_at(size_type index) {
        return data_[index];
    }

    const T& unsafe_at(size_type index) const {
        return data_[index];
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
        func(*const_cast<deque*>(this));
    }

#if TS_STL_SUPPORT_RW_LOCK
    template <typename Func>
    void with_read_lock(Func func) const {
        auto guard = acquire_read_lock();
        func(*this);
    }
#endif
};

// ==================== Deque 的 LockFree 特化版本 ====================

template <typename T>
class deque<T, LockPolicy::LockFree> {
private:
    std::deque<T> data_;

public:
    using value_type = T;
    using size_type = typename std::deque<T>::size_type;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::deque<T>::iterator;
    using const_iterator = typename std::deque<T>::const_iterator;

    deque() = default;

    explicit deque(size_type count) : data_(count) {}

    deque(size_type count, const T& value) : data_(count, value) {}

    template <typename InputIt>
    deque(InputIt first, InputIt last) : data_(first, last) {}

    deque(const deque& other) = default;
    deque& operator=(const deque& other) = default;
    deque(deque&& other) noexcept = default;
    deque& operator=(deque&& other) noexcept = default;

    reference operator[](size_type index) noexcept {
        return data_[index];
    }

    const_reference operator[](size_type index) const noexcept {
        return data_[index];
    }

    reference at(size_type index) {
        return data_.at(index);
    }

    const_reference at(size_type index) const {
        return data_.at(index);
    }

    T get(size_type index, const T& default_value = T()) const {
        if (index < data_.size()) {
            return data_[index];
        }
        return default_value;
    }

    void set(size_type index, const T& value) {
        if (index < data_.size()) {
            data_[index] = value;
        }
    }

    reference front() noexcept {
        return data_.front();
    }

    const_reference front() const noexcept {
        return data_.front();
    }

    reference back() noexcept {
        return data_.back();
    }

    const_reference back() const noexcept {
        return data_.back();
    }

    size_type size() const noexcept {
        return data_.size();
    }

    bool empty() const noexcept {
        return data_.empty();
    }

    void resize(size_type count) noexcept {
        data_.resize(count);
    }

    void resize(size_type count, const T& value) noexcept {
        data_.resize(count, value);
    }

    void clear() noexcept {
        data_.clear();
    }

    void shrink_to_fit() noexcept {
        data_.shrink_to_fit();
    }

    void push_back(const T& value) {
        data_.push_back(value);
    }

    void push_back(T&& value) {
        data_.push_back(std::move(value));
    }

    void pop_back() noexcept {
        if (!data_.empty()) {
            data_.pop_back();
        }
    }

    void push_front(const T& value) {
        data_.push_front(value);
    }

    void push_front(T&& value) {
        data_.push_front(std::move(value));
    }

    void pop_front() noexcept {
        if (!data_.empty()) {
            data_.pop_front();
        }
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
        return data_.back();
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        data_.emplace_front(std::forward<Args>(args)...);
        return data_.front();
    }

    operator std::deque<T>&() noexcept {
        return data_;
    }

    operator const std::deque<T>&() const noexcept {
        return data_;
    }

    std::deque<T>& get_unsafe() noexcept {
        return data_;
    }

    const std::deque<T>& get_unsafe() const noexcept {
        return data_;
    }
};

} // namespace ts_stl

#endif // TS_DEQUE_HPP
