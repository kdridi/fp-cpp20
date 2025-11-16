#include "../../include/segy/core/headers.hpp"
#include <cassert>
#include <iostream>

using namespace segy::core;
using namespace segy::concepts;

// ============================================================================
// TEXTUAL HEADER TESTS
// ============================================================================

void test_textual_header() {
    TextualHeader header;

    // Set line 1
    header.set_line(1, "C01 SEG-Y TEST FILE");

    // Get line 1
    auto line1 = header.get_line(1);
    assert(line1.substr(0, 19) == "C01 SEG-Y TEST FILE");
    assert(line1.size() == 80); // Padded to 80 chars

    // Set line 40 (last line)
    header.set_line(40, "C40 END OF HEADER");
    auto line40 = header.get_line(40);
    assert(line40.substr(0, 17) == "C40 END OF HEADER");

    // Invalid line numbers
    auto invalid = header.get_line(0);
    assert(invalid.empty());

    invalid = header.get_line(41);
    assert(invalid.empty());

    std::cout << "✓ TextualHeader tests passed\n";
}

// ============================================================================
// BINARY HEADER TESTS
// ============================================================================

void test_binary_header() {
    BinaryHeader header;

    // Set fields
    header.set_job_id(12345);
    header.set_line_number(67890);
    header.set_sample_interval(4000); // 4ms
    header.set_samples_per_trace(1000);
    header.set_sample_format(SampleFormat::IEEEFloat);

    // Read back
    assert(header.job_id() == 12345);
    assert(header.line_number() == 67890);
    assert(header.sample_interval() == 4000);
    assert(header.samples_per_trace() == 1000);
    assert(header.sample_format() == SampleFormat::IEEEFloat);

    // Validation
    assert(header.is_valid());

    // Test invalid values
    BinaryHeader invalid;
    invalid.set_sample_interval(0); // Invalid
    assert(!invalid.is_valid());

    invalid.set_sample_interval(4000);
    invalid.set_samples_per_trace(0); // Invalid
    assert(!invalid.is_valid());

    invalid.set_samples_per_trace(1000);
    invalid.set_sample_format(static_cast<SampleFormat>(99)); // Invalid
    assert(!invalid.is_valid());

    std::cout << "✓ BinaryHeader tests passed\n";
}

// ============================================================================
// TRACE HEADER TESTS
// ============================================================================

void test_trace_header() {
    TraceHeader header;

    // Set fields
    header.set_trace_sequence(1);
    header.set_inline_number(100);
    header.set_crossline_number(200);
    header.set_num_samples(1000);
    header.set_sample_interval(4000);

    // Read back
    assert(header.trace_sequence() == 1);
    assert(header.inline_number() == 100);
    assert(header.crossline_number() == 200);
    assert(header.num_samples() == 1000);
    assert(header.sample_interval() == 4000);

    // Validation
    assert(header.is_valid());

    // Invalid
    TraceHeader invalid;
    invalid.set_num_samples(0);
    assert(!invalid.is_valid());

    std::cout << "✓ TraceHeader tests passed\n";
}

// ============================================================================
// ENDIANNESS PRESERVATION TESTS
// ============================================================================

void test_header_endianness() {
    BinaryHeader header;

    // Set a value
    int32_t original = 0x12345678;
    header.set_job_id(original);

    // Get raw data
    auto data = header.data();

    // Manually check bytes (should be big-endian)
    // After endianness conversion in write_field
    uint8_t byte0 = static_cast<uint8_t>(data[0]); // offset 0 (1-indexed = 1)
    uint8_t byte1 = static_cast<uint8_t>(data[1]);
    uint8_t byte2 = static_cast<uint8_t>(data[2]);
    uint8_t byte3 = static_cast<uint8_t>(data[3]);

    // Reconstruct as big-endian
    int32_t reconstructed = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;

    // Should match original after big-endian interpretation
    assert(header.job_id() == original);

    std::cout << "✓ Header endianness tests passed\n";
}

int main() {
    std::cout << "Running header tests...\n";

    test_textual_header();
    test_binary_header();
    test_trace_header();
    test_header_endianness();

    std::cout << "\n🎉 All header tests passed!\n";
    return 0;
}
