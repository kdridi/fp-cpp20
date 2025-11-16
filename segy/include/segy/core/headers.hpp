#pragma once

#include <array>
#include <span>
#include <string>
#include <cstring>
#include "../concepts/segy_concepts.hpp"
#include "endian.hpp"

namespace segy::core {

using namespace concepts;
using namespace constants;

// ============================================================================
// TEXTUAL HEADER (3200 bytes EBCDIC/ASCII)
// ============================================================================

/// @brief SEG-Y Textual Header (40 lines of 80 characters)
/// @note Traditionally EBCDIC, modern files use ASCII
class TextualHeader {
    std::array<char, TEXTUAL_HEADER_SIZE> data_{};

public:
    constexpr TextualHeader() = default;

    /// @brief Construct from span (zero-copy view)
    explicit TextualHeader(std::span<const std::byte> bytes) {
        if (bytes.size() >= TEXTUAL_HEADER_SIZE) {
            std::memcpy(data_.data(), bytes.data(), TEXTUAL_HEADER_SIZE);
        }
    }

    /// @brief Get line (1-indexed, 1-40)
    constexpr std::string_view get_line(size_t line_num) const {
        if (line_num < 1 || line_num > 40) return {};
        const size_t offset = (line_num - 1) * 80;
        return std::string_view(data_.data() + offset, 80);
    }

    /// @brief Set line (1-indexed)
    constexpr void set_line(size_t line_num, std::string_view text) {
        if (line_num < 1 || line_num > 40) return;
        const size_t offset = (line_num - 1) * 80;
        const size_t len = std::min(text.size(), size_t(80));
        std::memcpy(data_.data() + offset, text.data(), len);
        // Pad with spaces
        if (len < 80) {
            std::memset(data_.data() + offset + len, ' ', 80 - len);
        }
    }

    /// @brief Get raw data
    std::span<const std::byte> data() const {
        return std::span(reinterpret_cast<const std::byte*>(data_.data()), TEXTUAL_HEADER_SIZE);
    }

    std::span<std::byte> data() {
        return std::span(reinterpret_cast<std::byte*>(data_.data()), TEXTUAL_HEADER_SIZE);
    }
};

// ============================================================================
// BINARY HEADER (400 bytes)
// ============================================================================

/// @brief SEG-Y Binary Header with type-safe field access
class BinaryHeader {
    std::array<std::byte, BINARY_HEADER_SIZE> data_{};

public:
    constexpr BinaryHeader() = default;

    /// @brief Construct from span (zero-copy view)
    explicit BinaryHeader(std::span<const std::byte> bytes) {
        if (bytes.size() >= BINARY_HEADER_SIZE) {
            std::memcpy(data_.data(), bytes.data(), BINARY_HEADER_SIZE);
        }
    }

    // Type-safe field accessors

    constexpr int32_t job_id() const {
        return extract_field<int32_t>(data_, JOB_ID_OFFSET);
    }
    constexpr void set_job_id(int32_t value) {
        write_field(data_, JOB_ID_OFFSET, value);
    }

    constexpr int32_t line_number() const {
        return extract_field<int32_t>(data_, LINE_NUMBER_OFFSET);
    }
    constexpr void set_line_number(int32_t value) {
        write_field(data_, LINE_NUMBER_OFFSET, value);
    }

    constexpr int32_t reel_number() const {
        return extract_field<int32_t>(data_, REEL_NUMBER_OFFSET);
    }
    constexpr void set_reel_number(int32_t value) {
        write_field(data_, REEL_NUMBER_OFFSET, value);
    }

    constexpr int16_t sample_interval() const {
        return extract_field<int16_t>(data_, SAMPLE_INTERVAL_OFFSET);
    }
    constexpr void set_sample_interval(int16_t value) {
        write_field(data_, SAMPLE_INTERVAL_OFFSET, value);
    }

    constexpr int16_t samples_per_trace() const {
        return extract_field<int16_t>(data_, SAMPLES_PER_TRACE_OFFSET);
    }
    constexpr void set_samples_per_trace(int16_t value) {
        write_field(data_, SAMPLES_PER_TRACE_OFFSET, value);
    }

    constexpr SampleFormat sample_format() const {
        return static_cast<SampleFormat>(extract_field<int16_t>(data_, SAMPLE_FORMAT_OFFSET));
    }
    constexpr void set_sample_format(SampleFormat format) {
        write_field(data_, SAMPLE_FORMAT_OFFSET, static_cast<int16_t>(format));
    }

    /// @brief Validate header fields
    constexpr bool is_valid() const {
        const auto si = sample_interval();
        const auto spt = samples_per_trace();
        const auto fmt = sample_format();

        return si >= MIN_SAMPLE_INTERVAL &&
               si <= MAX_SAMPLE_INTERVAL &&
               spt > 0 &&
               spt <= MAX_SAMPLES_PER_TRACE &&
               (static_cast<uint16_t>(fmt) >= 1 && static_cast<uint16_t>(fmt) <= 8);
    }

    std::span<const std::byte> data() const {
        return std::span(data_.data(), BINARY_HEADER_SIZE);
    }

    std::span<std::byte> data() {
        return std::span(data_.data(), BINARY_HEADER_SIZE);
    }
};

// ============================================================================
// TRACE HEADER (240 bytes)
// ============================================================================

/// @brief SEG-Y Trace Header with type-safe field access
class TraceHeader {
    std::array<std::byte, TRACE_HEADER_SIZE> data_{};

public:
    constexpr TraceHeader() = default;

    explicit TraceHeader(std::span<const std::byte> bytes) {
        if (bytes.size() >= TRACE_HEADER_SIZE) {
            std::memcpy(data_.data(), bytes.data(), TRACE_HEADER_SIZE);
        }
    }

    constexpr int32_t trace_sequence() const {
        return extract_field<int32_t>(data_, TRACE_SEQUENCE_OFFSET);
    }
    constexpr void set_trace_sequence(int32_t value) {
        write_field(data_, TRACE_SEQUENCE_OFFSET, value);
    }

    constexpr int32_t inline_number() const {
        return extract_field<int32_t>(data_, INLINE_NUMBER_OFFSET);
    }
    constexpr void set_inline_number(int32_t value) {
        write_field(data_, INLINE_NUMBER_OFFSET, value);
    }

    constexpr int32_t crossline_number() const {
        return extract_field<int32_t>(data_, CROSSLINE_NUMBER_OFFSET);
    }
    constexpr void set_crossline_number(int32_t value) {
        write_field(data_, CROSSLINE_NUMBER_OFFSET, value);
    }

    constexpr int16_t num_samples() const {
        return extract_field<int16_t>(data_, TRACE_SAMPLES_OFFSET);
    }
    constexpr void set_num_samples(int16_t value) {
        write_field(data_, TRACE_SAMPLES_OFFSET, value);
    }

    constexpr int16_t sample_interval() const {
        return extract_field<int16_t>(data_, TRACE_SAMPLE_INTERVAL_OFFSET);
    }
    constexpr void set_sample_interval(int16_t value) {
        write_field(data_, TRACE_SAMPLE_INTERVAL_OFFSET, value);
    }

    constexpr bool is_valid() const {
        const auto ns = num_samples();
        const auto si = sample_interval();
        return ns > 0 && ns <= MAX_SAMPLES_PER_TRACE &&
               si >= MIN_SAMPLE_INTERVAL && si <= MAX_SAMPLE_INTERVAL;
    }

    std::span<const std::byte> data() const {
        return std::span(data_.data(), TRACE_HEADER_SIZE);
    }

    std::span<std::byte> data() {
        return std::span(data_.data(), TRACE_HEADER_SIZE);
    }
};

} // namespace segy::core
