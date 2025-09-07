/*
 * Project: FrameKit
 * File: RollingBuffer.h
 * Author: George Gil
 * Date: 2025-08-22
 * License: MIT
 * Description:
 *   Fixed-capacity, thread-safe rolling (ring) buffer.
 *   - MPMC using std::mutex (for in-process threads, not interprocess).
 *   - Non-blocking: try_emplace / try_push / try_pop / try_peek.
 *   - Blocking: wait_emplace_for / wait_push_for / wait_pop_for (+ no-timeout variants).
 *   - No default-constructibility required for T (std::optional<T> slots).
 */

#pragma once

#include <array>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include <chrono>
#include <type_traits>
#include <utility>

namespace FrameKit::Containers {

template <typename T, std::size_t Capacity>
class RollingBuffer {
  static_assert(Capacity > 0, "RollingBuffer capacity must be greater than zero.");

public:
  using value_type      = T;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  // --------- construction ----------
  RollingBuffer() noexcept = default;
  RollingBuffer(const RollingBuffer&)            = delete;
  RollingBuffer& operator=(const RollingBuffer&) = delete;
  RollingBuffer(RollingBuffer&&)                 = delete;
  RollingBuffer& operator=(RollingBuffer&&)      = delete;

  // --------- capacity ----------
  [[nodiscard]] static constexpr size_type capacity() noexcept { return Capacity; }

  [[nodiscard]] size_type size() const noexcept {
    std::scoped_lock lock(m_mtx);
    return m_count;
  }

  [[nodiscard]] bool empty() const noexcept {
    std::scoped_lock lock(m_mtx);
    return m_count == 0;
  }

  [[nodiscard]] bool full() const noexcept {
    std::scoped_lock lock(m_mtx);
    return m_count == Capacity;
  }

  // Some legacy code expects this name:
  [[nodiscard]] bool fullCapacity() const noexcept { return full(); }

  // --------- modifiers (non-blocking) ----------
  template <class... Args>
  [[nodiscard]] bool try_emplace(Args&&... args) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
    if (!lock.owns_lock() || m_count == Capacity)
      return false;

    m_slots[m_head].emplace(std::forward<Args>(args)...);
    advance_head_unlocked();
    lock.unlock();
    m_cvNotEmpty.notify_one();
    return true;
  }

  [[nodiscard]] bool try_push(const T& item) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
    if (!lock.owns_lock() || m_count == Capacity)
      return false;

    m_slots[m_head].emplace(item);
    advance_head_unlocked();
    lock.unlock();
    m_cvNotEmpty.notify_one();
    return true;
  }

  [[nodiscard]] bool try_push(T&& item) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
    if (!lock.owns_lock() || m_count == Capacity)
      return false;

    m_slots[m_head].emplace(std::move(item));
    advance_head_unlocked();
    lock.unlock();
    m_cvNotEmpty.notify_one();
    return true;
  }

  [[nodiscard]] bool try_pop(T& out) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
    if (!lock.owns_lock() || m_count == 0)
      return false;

    out = std::move(*m_slots[m_tail]);
    m_slots[m_tail].reset();
    advance_tail_unlocked();
    lock.unlock();
    m_cvNotFull.notify_one();
    return true;
  }

  [[nodiscard]] std::optional<T> try_pop() {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
    if (!lock.owns_lock() || m_count == 0)
      return std::nullopt;

    std::optional<T> out;
    out.emplace(std::move(*m_slots[m_tail]));
    m_slots[m_tail].reset();
    advance_tail_unlocked();
    lock.unlock();
    m_cvNotFull.notify_one();
    return out;
  }

  // Peek without removing.
  [[nodiscard]] bool try_peek(T& out) const {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
    if (!lock.owns_lock() || m_count == 0)
      return false;

    out = *m_slots[m_tail]; // copy (or move if T is cheaply movable)
    return true;
  }

  // --------- modifiers (blocking with timeout) ----------
  template <class Rep, class Period, class... Args>
  [[nodiscard]] bool wait_emplace_for(const std::chrono::duration<Rep, Period>& timeout, Args&&... args) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx);
    if (!m_cvNotFull.wait_for(lock, timeout, [&]{ return m_count < Capacity; }))
      return false;

    m_slots[m_head].emplace(std::forward<Args>(args)...);
    advance_head_unlocked();
    lock.unlock();
    m_cvNotEmpty.notify_one();
    return true;
  }

  template <class Rep, class Period>
  [[nodiscard]] bool wait_push_for(const std::chrono::duration<Rep, Period>& timeout, const T& item) {
    return wait_emplace_for(timeout, item);
  }

  template <class Rep, class Period>
  [[nodiscard]] bool wait_push_for(const std::chrono::duration<Rep, Period>& timeout, T&& item) {
    return wait_emplace_for(timeout, std::move(item));
  }

  template <class Rep, class Period>
  [[nodiscard]] bool wait_pop_for(const std::chrono::duration<Rep, Period>& timeout, T& out) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx);
    if (!m_cvNotEmpty.wait_for(lock, timeout, [&]{ return m_count > 0; }))
      return false;

    out = std::move(*m_slots[m_tail]);
    m_slots[m_tail].reset();
    advance_tail_unlocked();
    lock.unlock();
    m_cvNotFull.notify_one();
    return true;
  }

  // --------- modifiers (blocking no-timeout) ----------
  template <class... Args>
  void wait_emplace(Args&&... args) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cvNotFull.wait(lock, [&]{ return m_count < Capacity; });
    m_slots[m_head].emplace(std::forward<Args>(args)...);
    advance_head_unlocked();
    lock.unlock();
    m_cvNotEmpty.notify_one();
  }

  void wait_push(const T& item) { wait_emplace(item); }
  void wait_push(T&& item)      { wait_emplace(std::move(item)); }

  void wait_pop(T& out) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cvNotEmpty.wait(lock, [&]{ return m_count > 0; });
    out = std::move(*m_slots[m_tail]);
    m_slots[m_tail].reset();
    advance_tail_unlocked();
    lock.unlock();
    m_cvNotFull.notify_one();
  }

  // --------- management ----------
  void clear() noexcept {
    FK_PROFILE_FUNCTION();
    std::scoped_lock lock(m_mtx);
    if (m_count > 0) {
      size_type idx = m_tail;
      for (size_type i = 0; i < m_count; ++i) {
        m_slots[idx].reset();
        idx = (idx + 1) % Capacity;
      }
    }
    m_head = 0;
    m_tail = 0;
    m_count = 0;
    // notify producers that space is available
    m_cvNotFull.notify_all();
  }

  // Remove all elements via pops into a sink functor (avoids copies if you want).
  template <typename Sink>
  void drain(Sink&& sink) {
    FK_PROFILE_FUNCTION();
    std::unique_lock<std::mutex> lock(m_mtx);
    while (m_count > 0) {
      sink(*m_slots[m_tail]);            // sink can take by const&, &&, etc.
      m_slots[m_tail].reset();
      advance_tail_unlocked();
    }
    lock.unlock();
    m_cvNotFull.notify_all();
  }

private:
  // Internal helpers — require m_mtx held.
  void advance_head_unlocked() noexcept {
    m_head = (m_head + 1) % Capacity;
    ++m_count;
  }

  void advance_tail_unlocked() noexcept {
    m_tail = (m_tail + 1) % Capacity;
    --m_count;
  }

  // State
  mutable std::mutex                      m_mtx;
  std::condition_variable                 m_cvNotEmpty;
  std::condition_variable                 m_cvNotFull;
  std::array<std::optional<T>, Capacity>  m_slots{};
  size_type                               m_head  = 0;  // next write
  size_type                               m_tail  = 0;  // next read
  size_type                               m_count = 0;  // number of valid items
};

} // namespace FrameKit::Containers
