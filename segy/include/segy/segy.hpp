#pragma once

/// @file segy.hpp
/// @brief Main header for SEG-Y library
/// @note SEG-Y Rev 1 compliant reader/writer
///
/// Features:
/// - Type-safe headers with compile-time validation
/// - Zero-copy field extraction
/// - Automatic endianness conversion
/// - IBM float <-> IEEE float conversion
/// - Optimized I/O with buffering
///
/// Example usage:
/// ```cpp
/// #include <segy/segy.hpp>
///
/// // Reading
/// segy::io::Reader<float> reader("seismic.segy");
/// auto trace = reader.read_trace(0);
/// auto samples = trace.samples();
///
/// // Writing
/// segy::core::BinaryHeader header;
/// header.set_sample_format(segy::concepts::SampleFormat::IEEEFloat);
/// header.set_samples_per_trace(1000);
/// header.set_sample_interval(4000); // 4ms
///
/// segy::io::Writer<float> writer("output.segy", textual, header);
/// writer.write_trace(trace);
/// ```

#include "concepts/segy_concepts.hpp"
#include "core/endian.hpp"
#include "core/headers.hpp"
#include "core/trace.hpp"
#include "io/reader.hpp"
#include "io/writer.hpp"

namespace segy {

// Re-export main types for convenience
using concepts::SampleFormat;
using concepts::Endianness;

using core::TextualHeader;
using core::BinaryHeader;
using core::TraceHeader;
using core::Trace;

using io::Reader;
using io::Writer;

} // namespace segy
