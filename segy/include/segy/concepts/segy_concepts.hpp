#pragma once

#include <concepts>
#include <cstdint>
#include <span>

namespace segy::concepts {

// ============================================================================
// COMPILE-TIME VALIDATION CONCEPTS
// ============================================================================

/// @concept Endianness
/// SEG-Y supports Big Endian (IBM) and Little Endian
enum class Endianness : uint8_t {
    BigEndian,
    LittleEndian
};

/// @concept SampleFormat
/// SEG-Y Rev 1 sample formats
enum class SampleFormat : uint16_t {
    IBMFloat = 1,        // 4-byte IBM floating point
    Int32 = 2,           // 4-byte two's complement integer
    Int16 = 3,           // 2-byte two's complement integer
    FixedPoint = 4,      // 4-byte fixed point with gain
    IEEEFloat = 5,       // 4-byte IEEE floating point
    Int8 = 8             // 1-byte two's complement integer
};

/// @concept ValidSampleFormat
/// Compile-time validation of sample format
template<SampleFormat F>
concept ValidSampleFormat =
    F == SampleFormat::IBMFloat ||
    F == SampleFormat::Int32 ||
    F == SampleFormat::Int16 ||
    F == SampleFormat::FixedPoint ||
    F == SampleFormat::IEEEFloat ||
    F == SampleFormat::Int8;

/// @concept ByteSwappable
/// Types that can be byte-swapped for endianness conversion
template<typename T>
concept ByteSwappable =
    std::is_trivially_copyable_v<T> &&
    (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

/// @concept ContiguousRange
/// For zero-copy operations
template<typename R>
concept ContiguousRange = requires(R r) {
    { std::span(r) } -> std::convertible_to<std::span<typename R::value_type>>;
};

// ============================================================================
// COMPILE-TIME CONSTANTS (SEG-Y Rev 1 specification)
// ============================================================================

namespace constants {
    // File header sizes (bytes)
    constexpr size_t TEXTUAL_HEADER_SIZE = 3200;
    constexpr size_t BINARY_HEADER_SIZE = 400;
    constexpr size_t TRACE_HEADER_SIZE = 240;

    // Binary header offsets (1-indexed as per SEG-Y spec)
    constexpr size_t JOB_ID_OFFSET = 1;
    constexpr size_t LINE_NUMBER_OFFSET = 5;
    constexpr size_t REEL_NUMBER_OFFSET = 9;
    constexpr size_t SAMPLE_INTERVAL_OFFSET = 17;
    constexpr size_t SAMPLES_PER_TRACE_OFFSET = 21;
    constexpr size_t SAMPLE_FORMAT_OFFSET = 25;

    // Trace header offsets
    constexpr size_t TRACE_SEQUENCE_OFFSET = 1;
    constexpr size_t INLINE_NUMBER_OFFSET = 189;
    constexpr size_t CROSSLINE_NUMBER_OFFSET = 193;
    constexpr size_t TRACE_SAMPLES_OFFSET = 115;
    constexpr size_t TRACE_SAMPLE_INTERVAL_OFFSET = 117;

    // Validation constraints
    constexpr size_t MAX_SAMPLES_PER_TRACE = 32767;
    constexpr size_t MIN_SAMPLE_INTERVAL = 1; // microseconds
    constexpr size_t MAX_SAMPLE_INTERVAL = 1000000; // 1 second
}

// ============================================================================
// COMPILE-TIME FIELD METADATA
// ============================================================================

template<size_t Offset, typename T, size_t Size = sizeof(T)>
struct FieldMetadata {
    static constexpr size_t offset = Offset;
    using type = T;
    static constexpr size_t size = Size;

    // Compile-time validation
    static_assert(Offset > 0, "SEG-Y uses 1-indexed offsets");
    static_assert(Size > 0, "Field size must be positive");
};

} // namespace segy::concepts
