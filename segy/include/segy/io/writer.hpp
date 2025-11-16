#pragma once

#include <fstream>
#include <vector>
#include <filesystem>
#include "../core/headers.hpp"
#include "../core/trace.hpp"

namespace segy::io {

// ============================================================================
// SEG-Y WRITER (Type-safe, buffered)
// ============================================================================

/// @brief SEG-Y file writer with type-safe API
template<typename SampleType = float>
class Writer {
    std::filesystem::path filepath_;
    std::ofstream file_;
    core::TextualHeader textual_header_;
    core::BinaryHeader binary_header_;
    size_t traces_written_ = 0;

public:
    Writer() = default;

    /// @brief Create new SEG-Y file for writing
    /// @param path Output file path
    /// @param textual Textual header (40 lines x 80 chars)
    /// @param binary Binary header with format info
    explicit Writer(const std::filesystem::path& path,
                   const core::TextualHeader& textual,
                   const core::BinaryHeader& binary)
        : filepath_(path)
        , file_(path, std::ios::binary | std::ios::trunc)
        , textual_header_(textual)
        , binary_header_(binary)
    {
        if (!file_) {
            throw std::runtime_error("Cannot create file: " + path.string());
        }

        if (!binary_header_.is_valid()) {
            throw std::runtime_error("Invalid binary header");
        }

        write_headers();
    }

    /// @brief Write single trace
    void write_trace(const core::Trace<SampleType>& trace) {
        auto data = trace.serialize(binary_header_.sample_format());
        file_.write(reinterpret_cast<const char*>(data.data()), data.size());
        traces_written_++;
    }

    /// @brief Write multiple traces
    void write_traces(const std::vector<core::Trace<SampleType>>& traces) {
        for (const auto& trace : traces) {
            write_trace(trace);
        }
    }

    /// @brief Get number of traces written
    size_t traces_written() const { return traces_written_; }

    /// @brief Flush and close file
    void close() {
        file_.flush();
        file_.close();
    }

    /// @brief Destructor ensures file is closed
    ~Writer() {
        if (file_.is_open()) {
            close();
        }
    }

private:
    void write_headers() {
        // Write textual header
        auto textual_data = textual_header_.data();
        file_.write(reinterpret_cast<const char*>(textual_data.data()), textual_data.size());

        // Write binary header
        auto binary_data = binary_header_.data();
        file_.write(reinterpret_cast<const char*>(binary_data.data()), binary_data.size());
    }
};

} // namespace segy::io
