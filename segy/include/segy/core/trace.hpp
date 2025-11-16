#pragma once

#include <vector>
#include <span>
#include <memory>
#include "headers.hpp"
#include "endian.hpp"

namespace segy::core {

// ============================================================================
// TRACE DATA (Type-safe sample access)
// ============================================================================

/// @brief SEG-Y Trace with header and samples
/// @tparam SampleType Runtime sample type (float for IEEE, int32_t, etc.)
template<typename SampleType = float>
class Trace {
    TraceHeader header_;
    std::vector<SampleType> samples_;

public:
    Trace() = default;

    /// @brief Construct with header and pre-allocated samples
    explicit Trace(const TraceHeader& header, size_t num_samples = 0)
        : header_(header)
        , samples_(num_samples > 0 ? num_samples : header.num_samples())
    {}

    /// @brief Parse trace from binary data (zero-copy for header, copy for samples)
    /// @param data Binary data starting at trace header
    /// @param format Sample format from binary header
    static Trace parse(std::span<const std::byte> data, SampleFormat format) {
        if (data.size() < TRACE_HEADER_SIZE) {
            return Trace{};
        }

        TraceHeader header(data.subspan(0, TRACE_HEADER_SIZE));
        const size_t num_samples = header.num_samples();
        const size_t data_offset = TRACE_HEADER_SIZE;

        Trace trace(header, num_samples);

        // Parse samples based on format
        std::span<const std::byte> sample_data = data.subspan(data_offset);

        switch (format) {
            case SampleFormat::IBMFloat: {
                for (size_t i = 0; i < num_samples; ++i) {
                    uint32_t ibm = extract_field<uint32_t>(sample_data, i * 4 + 1);
                    trace.samples_[i] = static_cast<SampleType>(ibm_to_ieee(ibm));
                }
                break;
            }
            case SampleFormat::IEEEFloat: {
                for (size_t i = 0; i < num_samples; ++i) {
                    float val = extract_field<float>(sample_data, i * 4 + 1);
                    trace.samples_[i] = static_cast<SampleType>(val);
                }
                break;
            }
            case SampleFormat::Int32: {
                for (size_t i = 0; i < num_samples; ++i) {
                    int32_t val = extract_field<int32_t>(sample_data, i * 4 + 1);
                    trace.samples_[i] = static_cast<SampleType>(val);
                }
                break;
            }
            case SampleFormat::Int16: {
                for (size_t i = 0; i < num_samples; ++i) {
                    int16_t val = extract_field<int16_t>(sample_data, i * 2 + 1);
                    trace.samples_[i] = static_cast<SampleType>(val);
                }
                break;
            }
            case SampleFormat::Int8: {
                for (size_t i = 0; i < num_samples; ++i) {
                    int8_t val = static_cast<int8_t>(sample_data[i]);
                    trace.samples_[i] = static_cast<SampleType>(val);
                }
                break;
            }
            default:
                // Unsupported format
                break;
        }

        return trace;
    }

    /// @brief Serialize trace to binary (header + samples)
    std::vector<std::byte> serialize(SampleFormat format) const {
        const size_t sample_size = sample_byte_size(format);
        const size_t total_size = TRACE_HEADER_SIZE + samples_.size() * sample_size;

        std::vector<std::byte> data(total_size);

        // Copy header
        std::memcpy(data.data(), header_.data().data(), TRACE_HEADER_SIZE);

        // Write samples
        std::span<std::byte> sample_data = std::span(data).subspan(TRACE_HEADER_SIZE);

        switch (format) {
            case SampleFormat::IBMFloat: {
                for (size_t i = 0; i < samples_.size(); ++i) {
                    uint32_t ibm = ieee_to_ibm(static_cast<float>(samples_[i]));
                    write_field(sample_data, i * 4 + 1, ibm);
                }
                break;
            }
            case SampleFormat::IEEEFloat: {
                for (size_t i = 0; i < samples_.size(); ++i) {
                    write_field(sample_data, i * 4 + 1, static_cast<float>(samples_[i]));
                }
                break;
            }
            case SampleFormat::Int32: {
                for (size_t i = 0; i < samples_.size(); ++i) {
                    write_field(sample_data, i * 4 + 1, static_cast<int32_t>(samples_[i]));
                }
                break;
            }
            case SampleFormat::Int16: {
                for (size_t i = 0; i < samples_.size(); ++i) {
                    write_field(sample_data, i * 2 + 1, static_cast<int16_t>(samples_[i]));
                }
                break;
            }
            case SampleFormat::Int8: {
                for (size_t i = 0; i < samples_.size(); ++i) {
                    sample_data[i] = static_cast<std::byte>(static_cast<int8_t>(samples_[i]));
                }
                break;
            }
            default:
                break;
        }

        return data;
    }

    // Accessors
    constexpr const TraceHeader& header() const { return header_; }
    constexpr TraceHeader& header() { return header_; }

    constexpr std::span<const SampleType> samples() const { return samples_; }
    constexpr std::span<SampleType> samples() { return samples_; }

    constexpr size_t num_samples() const { return samples_.size(); }

private:
    static constexpr size_t sample_byte_size(SampleFormat format) {
        switch (format) {
            case SampleFormat::IBMFloat: return 4;
            case SampleFormat::Int32: return 4;
            case SampleFormat::Int16: return 2;
            case SampleFormat::FixedPoint: return 4;
            case SampleFormat::IEEEFloat: return 4;
            case SampleFormat::Int8: return 1;
            default: return 4;
        }
    }
};

} // namespace segy::core
