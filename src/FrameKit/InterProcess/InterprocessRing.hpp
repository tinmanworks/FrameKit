/*
 * Project: FrameKit
 * File: InterprocessRing.hpp
 * Author: George Gil
 * Date: 2025-08-22
 * License: MIT
 * Description:
 *   Interprocess-safe single-producer/single-consumer (SPSC) ring buffer.
 *   - Trivially-copyable T only (POD-friendly for shared memory).
 *   - Power-of-two capacity for fast masking.
 *   - Acquire/Release atomics; no mutexes/condvars.
 *   - Cache-line padding to minimize false sharing.
 */

#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace FrameKit::IPC {

// SPSC ring with power-of-two capacity.
// Notes:
//  - Uses monotonic counters for head/tail (wraparound is fine with size_t).
//  - Intended for one producer process/thread and one consumer process/thread.
template <typename T, std::size_t N>
struct alignas(64) SpscRing {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    static_assert((N & (N - 1)) == 0, "N must be a power of two");
    static_assert(N > 1, "N must be > 1");

    static constexpr std::size_t mask = N - 1;

    // --- Data buffer ---
    T buf[N];

    // --- Producer index (head) on its own cache line ---
    alignas(64) std::atomic_size_t head{0};
    // padding to keep consumer index off the same line
    alignas(64) std::atomic_size_t tail{0};

    // Non-blocking push (copy). Returns false if full.
    bool try_push(const T& v) {
        auto h = head.load(std::memory_order_relaxed);
        auto t = tail.load(std::memory_order_acquire);
        if (((h + 1) & mask) == (t & mask)) return false; // full
        buf[h & mask] = v;
        head.store(h + 1, std::memory_order_release);
        return true;
    }

    // Non-blocking push (move). Returns false if full.
    bool try_push(T&& v) {
        auto h = head.load(std::memory_order_relaxed);
        auto t = tail.load(std::memory_order_acquire);
        if (((h + 1) & mask) == (t & mask)) return false; // full
        buf[h & mask] = v; // trivially copyable -> move == copy
        head.store(h + 1, std::memory_order_release);
        return true;
    }

    // Non-blocking pop. Returns false if empty.
    bool try_pop(T& out) {
        auto t = tail.load(std::memory_order_relaxed);
        auto h = head.load(std::memory_order_acquire);
        if ((t & mask) == (h & mask)) return false; // empty
        out = buf[t & mask];
        tail.store(t + 1, std::memory_order_release);
        return true;
    }

    // Status helpers
    bool empty() const {
        auto t = tail.load(std::memory_order_acquire);
        auto h = head.load(std::memory_order_acquire);
        return (t & mask) == (h & mask);
    }

    bool full() const {
        auto h = head.load(std::memory_order_acquire);
        auto t = tail.load(std::memory_order_acquire);
        return ((h + 1) & mask) == (t & mask);
    }

    // Approximate size (may race but good for diagnostics/backpressure).
    std::size_t approx_size() const {
        auto h = head.load(std::memory_order_acquire);
        auto t = tail.load(std::memory_order_acquire);
        return (h - t) & mask;
    }

    static constexpr std::size_t capacity() { return N; }
};

} // namespace FrameKit::IPC
