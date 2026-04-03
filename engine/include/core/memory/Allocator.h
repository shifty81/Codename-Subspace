#pragma once

#include <array>
#include <cstddef>

namespace subspace {

/// Abstract base class for all engine allocators.
class Allocator {
public:
    virtual ~Allocator() = default;

    /// Allocate @p size bytes aligned to @p align (must be a power of two).
    /// Returns nullptr on failure.
    virtual void* Allocate(std::size_t size,
                           std::size_t align = alignof(std::max_align_t)) = 0;

    /// Return a previously allocated block to this allocator.
    /// Passing nullptr is a no-op.
    virtual void Deallocate(void* ptr) = 0;
};

// ---------------------------------------------------------------------------
// LinearAllocator
// ---------------------------------------------------------------------------

/// Bump/linear allocator backed by an externally-owned memory region.
///
/// Allocation is O(1); individual deallocation is not supported.
/// All memory is reclaimed at once via Reset().
///
/// Typical use: per-frame scratch memory.
class LinearAllocator : public Allocator {
public:
    /// Construct over an externally-owned backing buffer.
    /// @param buffer   Raw memory region.
    /// @param capacity Total size in bytes.
    LinearAllocator(void* buffer, std::size_t capacity) noexcept
        : _buffer(static_cast<std::byte*>(buffer))
        , _capacity(capacity)
        , _offset(0) {}

    void* Allocate(std::size_t size,
                   std::size_t align = alignof(std::max_align_t)) override {
        // Align the current offset up
        std::size_t cur = _offset;
        std::size_t pad = (align - (cur & (align - 1))) & (align - 1);
        std::size_t next = cur + pad + size;
        if (next > _capacity) return nullptr;
        _offset = next;
        return _buffer + cur + pad;
    }

    /// No-op: individual deallocation is not supported.
    void Deallocate(void* /*ptr*/) override {}

    /// Reset the offset to zero — logically frees all allocations.
    void Reset() noexcept { _offset = 0; }

    /// Bytes currently occupied (including alignment padding).
    std::size_t Used()     const noexcept { return _offset; }

    /// Total capacity of the backing buffer in bytes.
    std::size_t Capacity() const noexcept { return _capacity; }

private:
    std::byte*  _buffer   = nullptr;
    std::size_t _capacity = 0;
    std::size_t _offset   = 0;
};

// ---------------------------------------------------------------------------
// PoolAllocator
// ---------------------------------------------------------------------------

/// Fixed-size pool allocator using a stack-based free list.
///
/// O(1) alloc and free; capacity is fixed at compile time.
/// All slots have size and alignment of type T.
///
/// @tparam T  Object type that defines slot size/alignment.
/// @tparam N  Maximum number of live objects at any given time.
template<typename T, std::size_t N>
class PoolAllocator : public Allocator {
public:
    PoolAllocator() noexcept {
        for (std::size_t i = 0; i < N; ++i)
            _freeStack[i] = &_pool[i];
        _freeTop = N;
    }

    void* Allocate(std::size_t size, std::size_t /*align*/ = alignof(T)) override {
        if (size > sizeof(T) || _freeTop == 0) return nullptr;
        return _freeStack[--_freeTop];
    }

    void Deallocate(void* ptr) override {
        if (!ptr || _freeTop >= N) return;
        _freeStack[_freeTop++] = static_cast<Slot*>(ptr);
    }

    /// Number of free slots remaining.
    std::size_t FreeCount() const noexcept { return _freeTop; }

    /// Total capacity.
    static constexpr std::size_t Capacity() noexcept { return N; }

private:
    struct alignas(T) Slot { std::byte data[sizeof(T)]; };

    std::array<Slot,  N> _pool{};
    std::array<Slot*, N> _freeStack{};
    std::size_t          _freeTop = 0;
};

} // namespace subspace
