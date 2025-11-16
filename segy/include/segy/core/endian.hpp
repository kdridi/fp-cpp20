#pragma once

#include <bit>
#include <cstdint>
#include <concepts>
#include "../concepts/segy_concepts.hpp"

namespace segy::core {

// ============================================================================
// COMPILE-TIME ENDIANNESS CONVERSION (Zero-cost)
// ============================================================================

/// @brief Byte swap for endianness conversion
/// @note constexpr = can be evaluated at compile-time if inputs are constant
template<concepts::ByteSwappable T>
constexpr T byte_swap(T value) noexcept {
    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        return static_cast<T>(
            ((value & 0x00FF) << 8) |
            ((value & 0xFF00) >> 8)
        );
    } else if constexpr (sizeof(T) == 4) {
        return static_cast<T>(
            ((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) << 8)  |
            ((value & 0x00FF0000) >> 8)  |
            ((value & 0xFF000000) >> 24)
        );
    } else if constexpr (sizeof(T) == 8) {
        return static_cast<T>(
            ((value & 0x00000000000000FF) << 56) |
            ((value & 0x000000000000FF00) << 40) |
            ((value & 0x0000000000FF0000) << 24) |
            ((value & 0x00000000FF000000) << 8)  |
            ((value & 0x000000FF00000000) >> 8)  |
            ((value & 0x0000FF0000000000) >> 24) |
            ((value & 0x00FF000000000000) >> 40) |
            ((value & 0xFF00000000000000) >> 56)
        );
    }
}

// Compile-time tests
static_assert(byte_swap<uint16_t>(0x1234) == 0x3412);
static_assert(byte_swap<uint32_t>(0x12345678) == 0x78563412);

/// @brief Convert from big-endian to native
template<concepts::ByteSwappable T>
constexpr T from_big_endian(T value) noexcept {
    if constexpr (std::endian::native == std::endian::big) {
        return value;
    } else {
        return byte_swap(value);
    }
}

/// @brief Convert from native to big-endian
template<concepts::ByteSwappable T>
constexpr T to_big_endian(T value) noexcept {
    return from_big_endian(value); // Symmetric operation
}

// ============================================================================
// IBM FLOAT CONVERSION (SEG-Y uses IBM floating point)
// ============================================================================

/// @brief Convert IBM float (32-bit) to IEEE float
/// @note SEG-Y Rev 1 default format
constexpr float ibm_to_ieee(uint32_t ibm) noexcept {
    if (ibm == 0) return 0.0f;

    // IBM format: SEEEEEEE MMMMMMMM MMMMMMMM MMMMMMMM
    // S = sign (1 bit)
    // E = exponent (7 bits, base 16, excess 64)
    // M = mantissa (24 bits, normalized 0.1 to 1.0)

    int32_t sign = (ibm >> 31) ? -1 : 1;
    int32_t exponent = ((ibm >> 24) & 0x7F) - 64;
    uint32_t mantissa = ibm & 0x00FFFFFF;

    // Convert base-16 exponent to base-2
    // IBM: fraction * 16^exp
    // IEEE: fraction * 2^exp
    float value = static_cast<float>(mantissa) / 16777216.0f; // 2^24
    value *= sign;

    // Scale by 16^exponent = 2^(4*exponent)
    for (int i = 0; i < exponent * 4; ++i) {
        value *= 2.0f;
    }
    for (int i = 0; i > exponent * 4; --i) {
        value /= 2.0f;
    }

    return value;
}

/// @brief Convert IEEE float to IBM float
constexpr uint32_t ieee_to_ibm(float ieee) noexcept {
    if (ieee == 0.0f) return 0;

    uint32_t sign = (ieee < 0) ? 0x80000000 : 0;
    float abs_val = (ieee < 0) ? -ieee : ieee;

    // Find exponent (base 16)
    int exp = 0;
    while (abs_val >= 1.0f) {
        abs_val /= 16.0f;
        exp++;
    }
    while (abs_val < 0.0625f) { // 1/16
        abs_val *= 16.0f;
        exp--;
    }

    exp += 64; // Excess-64 notation
    uint32_t mantissa = static_cast<uint32_t>(abs_val * 16777216.0f); // 2^24

    return sign | ((static_cast<uint32_t>(exp) & 0x7F) << 24) | (mantissa & 0x00FFFFFF);
}

// ============================================================================
// ZERO-COPY FIELD EXTRACTION
// ============================================================================

/// @brief Extract field from binary data with automatic endianness conversion
template<typename T, concepts::Endianness E = concepts::Endianness::BigEndian>
    requires concepts::ByteSwappable<T>
constexpr T extract_field(std::span<const std::byte> data, size_t offset_1indexed) noexcept {
    const size_t offset = offset_1indexed - 1; // Convert to 0-indexed

    T value;
    std::memcpy(&value, &data[offset], sizeof(T));

    if constexpr (E == concepts::Endianness::BigEndian) {
        return from_big_endian(value);
    } else {
        return value;
    }
}

/// @brief Write field to binary data with automatic endianness conversion
template<typename T, concepts::Endianness E = concepts::Endianness::BigEndian>
    requires concepts::ByteSwappable<T>
constexpr void write_field(std::span<std::byte> data, size_t offset_1indexed, T value) noexcept {
    const size_t offset = offset_1indexed - 1;

    if constexpr (E == concepts::Endianness::BigEndian) {
        value = to_big_endian(value);
    }

    std::memcpy(&data[offset], &value, sizeof(T));
}

} // namespace segy::core
