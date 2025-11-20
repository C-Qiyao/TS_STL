#pragma once

#ifndef TS_STL_HPP
#define TS_STL_HPP

// 包含基础组件（锁和容器基类）
#include "ts_stl_base.hpp"

// 包含具体容器实现
#include "ts_vector.hpp"
#include "ts_list.hpp"
#include "ts_map.hpp"

namespace ts_stl {

// ==================== 便利类型别名 ====================

// 使用互斥锁的线程安全vector
template <typename T>
using vectorMutex = vector<T, LockPolicy::Mutex>;

#if TS_STL_SUPPORT_RW_LOCK
// 使用读写锁的线程安全vector（仅C++17及以上）
template <typename T>
using vectorRW = vector<T, LockPolicy::ReadWrite>;
#endif

// 使用自旋锁的线程安全vector
template <typename T>
using vectorSpinLock = vector<T, LockPolicy::SpinLock>;

// 使用无锁策略的线程安全vector（极限性能，需要外部同步）
template <typename T>
using vectorLockFree = vector<T, LockPolicy::LockFree>;

// 使用互斥锁的线程安全list
template <typename T>
using listMutex = list<T, LockPolicy::Mutex>;

// 使用自旋锁的线程安全list
template <typename T>
using listSpinLock = list<T, LockPolicy::SpinLock>;

// 使用无锁策略的线程安全list（极限性能，需要外部同步）
template <typename T>
using listLockFree = list<T, LockPolicy::LockFree>;

#if TS_STL_SUPPORT_RW_LOCK
// 使用读写锁的线程安全list（仅C++17及以上）
template <typename T>
using listRW = list<T, LockPolicy::ReadWrite>;
#endif

// ==================== Map 类型别名 ====================

// 使用互斥锁的线程安全map
template <typename Key, typename T, typename Compare = std::less<Key>>
using mapMutex = map<Key, T, Compare, LockPolicy::Mutex>;

#if TS_STL_SUPPORT_RW_LOCK
// 使用读写锁的线程安全map（仅C++17及以上）
template <typename Key, typename T, typename Compare = std::less<Key>>
using mapRW = map<Key, T, Compare, LockPolicy::ReadWrite>;
#endif

// 使用自旋锁的线程安全map
template <typename Key, typename T, typename Compare = std::less<Key>>
using mapSpinLock = map<Key, T, Compare, LockPolicy::SpinLock>;

// 使用无锁策略的线程安全map（极限性能，需要外部同步）
template <typename Key, typename T, typename Compare = std::less<Key>>
using mapLockFree = map<Key, T, Compare, LockPolicy::LockFree>;

// ==================== 兼容性别名 - 与旧API保持兼容 ====================

template <typename T, LockPolicy Policy = LockPolicy::Mutex>
using ThreadSafeVector = vector<T, Policy>;

template <typename T>
using ThreadSafeVectorMutex = vectorMutex<T>;

template <typename T>
using ThreadSafeVectorSpinLock = vectorSpinLock<T>;

template <typename T>
using ThreadSafeVectorLockFree = vectorLockFree<T>;

#if TS_STL_SUPPORT_RW_LOCK
template <typename T>
using ThreadSafeVectorRW = vectorRW<T>;
#endif

template <typename T, LockPolicy Policy = LockPolicy::Mutex>
using ThreadSafeList = list<T, Policy>;

template <typename T>
using ThreadSafeListMutex = listMutex<T>;

template <typename T>
using ThreadSafeListSpinLock = listSpinLock<T>;

template <typename T>
using ThreadSafeListLockFree = listLockFree<T>;

#if TS_STL_SUPPORT_RW_LOCK
template <typename T>
using ThreadSafeListRW = listRW<T>;
#endif

template <typename Key, typename T, typename Compare = std::less<Key>, LockPolicy Policy = LockPolicy::Mutex>
using ThreadSafeMap = map<Key, T, Compare, Policy>;

template <typename Key, typename T, typename Compare = std::less<Key>>
using ThreadSafeMapMutex = mapMutex<Key, T, Compare>;

template <typename Key, typename T, typename Compare = std::less<Key>>
using ThreadSafeMapSpinLock = mapSpinLock<Key, T, Compare>;

template <typename Key, typename T, typename Compare = std::less<Key>>
using ThreadSafeMapLockFree = mapLockFree<Key, T, Compare>;

#if TS_STL_SUPPORT_RW_LOCK
template <typename Key, typename T, typename Compare = std::less<Key>>
using ThreadSafeMapRW = mapRW<Key, T, Compare>;
#endif

} // namespace ts_stl

#endif // TS_STL_HPP
