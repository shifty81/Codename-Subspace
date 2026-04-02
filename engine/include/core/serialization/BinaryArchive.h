#pragma once

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace subspace {

/// Abstract base for all serialization archives.
class Archive {
public:
    virtual ~Archive() = default;
    virtual bool IsReading() const noexcept = 0;
    bool IsWriting() const noexcept { return !IsReading(); }
};

/// In-memory binary archive.
///
/// In write mode the archive accumulates bytes in an internal buffer.
/// In read mode it consumes bytes from a caller-supplied span.
///
/// Designed for C++17: uses SFINAE via enable_if instead of C++20 concepts.
///
/// Usage (write):
/// @code
///   BinaryArchive ar;
///   int x = 42;
///   ar.Serialize(x);
///   const auto& bytes = ar.Data(); // send / store bytes
/// @endcode
///
/// Usage (read):
/// @code
///   const std::vector<std::byte>& src = /* ... */;
///   BinaryArchive ar(src.data(), src.size());
///   int x = 0;
///   ar.Serialize(x); // x == 42
/// @endcode
class BinaryArchive : public Archive {
public:
    /// Construct a write-mode archive with an empty internal buffer.
    BinaryArchive() noexcept : _reading(false) {}

    /// Construct a read-mode archive over existing byte data.
    /// @param data  Pointer to the source bytes (must remain valid for the
    ///              lifetime of the archive).
    /// @param size  Number of bytes available.
    BinaryArchive(const std::byte* data, std::size_t size)
        : _buffer(data, data + size), _reading(true) {}

    bool IsReading() const noexcept override { return _reading; }

    /// Read or write a trivially-copyable value.
    ///
    /// In write mode the bytes of @p val are appended to the internal buffer.
    /// In read mode @p val is filled from the next sizeof(T) bytes.
    ///
    /// @tparam T  A trivially-copyable type.
    /// @param val Value to serialize (in/out).
    /// @throws std::out_of_range if reading past the end of the buffer.
    template<typename T,
             typename = typename std::enable_if<
                 std::is_trivially_copyable<T>::value>::type>
    void Serialize(T& val) {
        if (_reading) {
            if (_readPos + sizeof(T) > _buffer.size())
                throw std::out_of_range(
                    "BinaryArchive: read past end of buffer");
            std::memcpy(&val, _buffer.data() + _readPos, sizeof(T));
            _readPos += sizeof(T);
        } else {
            const auto* src = reinterpret_cast<const std::byte*>(&val);
            _buffer.insert(_buffer.end(), src, src + sizeof(T));
        }
    }

    /// Serialize a std::string as [uint32_t length][chars].
    void Serialize(std::string& s) {
        if (_reading) {
            uint32_t len = 0;
            Serialize(len);
            if (_readPos + len > _buffer.size())
                throw std::out_of_range(
                    "BinaryArchive: string read past end of buffer");
            s.assign(reinterpret_cast<const char*>(
                         _buffer.data() + _readPos), len);
            _readPos += len;
        } else {
            auto len = static_cast<uint32_t>(s.size());
            Serialize(len);
            const auto* src = reinterpret_cast<const std::byte*>(s.data());
            _buffer.insert(_buffer.end(), src, src + len);
        }
    }

    /// Access the written byte buffer (write mode only).
    const std::vector<std::byte>& Data() const noexcept { return _buffer; }

    /// Number of bytes consumed so far (read mode).
    std::size_t ReadPosition() const noexcept { return _readPos; }

    /// Total bytes in the buffer.
    std::size_t Size() const noexcept { return _buffer.size(); }

private:
    std::vector<std::byte> _buffer;
    std::size_t            _readPos = 0;
    bool                   _reading;
};

} // namespace subspace
