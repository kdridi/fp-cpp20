#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <span>
#include <filesystem>
#include "../core/headers.hpp"
#include "../core/trace.hpp"

namespace segy::io {

// ============================================================================
// SEG-Y READER (Zero-copy optimized)
// ============================================================================

/// @brief SEG-Y file reader with type-safe API
/// @note Uses memory mapping for large files (future optimization)
template<typename SampleType = float>
class Reader {
    std::filesystem::path filepath_;
    std::ifstream file_;

    core::TextualHeader textual_header_;
    core::BinaryHeader binary_header_;

    size_t num_traces_ = 0;
    size_t trace_size_bytes_ = 0;
    size_t data_start_offset_ = 0;

public:
    Reader() = default;

    /// @brief Open SEG-Y file for reading
    /// @throws std::runtime_error if file invalid
    explicit Reader(const std::filesystem::path& path)
        : filepath_(path)
        , file_(path, std::ios::binary)
    {
        if (!file_) {
            throw std::runtime_error("Cannot open file: " + path.string());
        }

        parse_headers();
        calculate_geometry();
    }

    /// @brief Get textual header
    const core::TextualHeader& textual_header() const { return textual_header_; }

    /// @brief Get binary header
    const core::BinaryHeader& binary_header() const { return binary_header_; }

    /// @brief Get number of traces in file
    size_t num_traces() const { return num_traces_; }

    /// @brief Read trace at index (0-indexed)
    core::Trace<SampleType> read_trace(size_t index) {
        if (index >= num_traces_) {
            throw std::out_of_range("Trace index out of range");
        }

        const size_t offset = data_start_offset_ + index * trace_size_bytes_;
        file_.seekg(static_cast<std::streamoff>(offset));

        std::vector<std::byte> trace_data(trace_size_bytes_);
        file_.read(reinterpret_cast<char*>(trace_data.data()), static_cast<std::streamsize>(trace_size_bytes_));

        return core::Trace<SampleType>::parse(trace_data, binary_header_.sample_format());
    }

    /// @brief Read all traces (careful with memory!)
    std::vector<core::Trace<SampleType>> read_all_traces() {
        std::vector<core::Trace<SampleType>> traces;
        traces.reserve(num_traces_);

        for (size_t i = 0; i < num_traces_; ++i) {
            traces.push_back(read_trace(i));
        }

        return traces;
    }

    /// @brief Read range of traces [start, end)
    std::vector<core::Trace<SampleType>> read_traces(size_t start, size_t end) {
        if (end > num_traces_) end = num_traces_;
        if (start >= end) return {};

        std::vector<core::Trace<SampleType>> traces;
        traces.reserve(end - start);

        for (size_t i = start; i < end; ++i) {
            traces.push_back(read_trace(i));
        }

        return traces;
    }

    /// @brief Validate file structure
    bool is_valid() const {
        return binary_header_.is_valid() && num_traces_ > 0;
    }

private:
    void parse_headers() {
        // Read textual header (3200 bytes)
        std::vector<std::byte> textual_data(concepts::constants::TEXTUAL_HEADER_SIZE);
        file_.read(reinterpret_cast<char*>(textual_data.data()), static_cast<std::streamsize>(textual_data.size()));
        textual_header_ = core::TextualHeader(textual_data);

        // Read binary header (400 bytes)
        std::vector<std::byte> binary_data(concepts::constants::BINARY_HEADER_SIZE);
        file_.read(reinterpret_cast<char*>(binary_data.data()), static_cast<std::streamsize>(binary_data.size()));
        binary_header_ = core::BinaryHeader(binary_data);

        if (!binary_header_.is_valid()) {
            throw std::runtime_error("Invalid binary header");
        }

        data_start_offset_ = concepts::constants::TEXTUAL_HEADER_SIZE +
                            concepts::constants::BINARY_HEADER_SIZE;
    }

    void calculate_geometry() {
        const auto samples_per_trace = binary_header_.samples_per_trace();
        const auto format = binary_header_.sample_format();

        size_t sample_size = 4; // Default
        switch (format) {
            case concepts::SampleFormat::IBMFloat:
            case concepts::SampleFormat::Int32:
            case concepts::SampleFormat::IEEEFloat:
            case concepts::SampleFormat::FixedPoint:
                sample_size = 4;
                break;
            case concepts::SampleFormat::Int16:
                sample_size = 2;
                break;
            case concepts::SampleFormat::Int8:
                sample_size = 1;
                break;
        }

        trace_size_bytes_ = concepts::constants::TRACE_HEADER_SIZE +
                           static_cast<size_t>(samples_per_trace) * sample_size;

        // Calculate number of traces from file size
        file_.seekg(0, std::ios::end);
        const size_t file_size = static_cast<size_t>(file_.tellg());
        file_.seekg(0);

        const size_t data_size = file_size - data_start_offset_;
        num_traces_ = data_size / trace_size_bytes_;
    }
};

} // namespace segy::io
