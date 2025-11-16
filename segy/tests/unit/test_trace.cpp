#include "../../include/segy/core/trace.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace segy::core;
using namespace segy::concepts;

// ============================================================================
// TRACE TESTS
// ============================================================================

void test_trace_construction() {
    TraceHeader header;
    header.set_num_samples(100);
    header.set_trace_sequence(1);

    Trace<float> trace(header, 100);

    assert(trace.num_samples() == 100);
    assert(trace.header().trace_sequence() == 1);

    // Modify samples
    auto samples = trace.samples();
    for (size_t i = 0; i < samples.size(); ++i) {
        samples[i] = static_cast<float>(i);
    }

    // Verify
    for (size_t i = 0; i < samples.size(); ++i) {
        assert(std::abs(samples[i] - static_cast<float>(i)) < 0.001f);
    }

    std::cout << "✓ Trace construction tests passed\n";
}

void test_trace_serialization_ieee() {
    // Create trace
    TraceHeader header;
    header.set_num_samples(10);
    header.set_trace_sequence(42);
    header.set_inline_number(100);
    header.set_sample_interval(4000);

    Trace<float> trace(header, 10);
    auto samples = trace.samples();
    for (size_t i = 0; i < 10; ++i) {
        samples[i] = static_cast<float>(i) * 1.5f;
    }

    // Serialize
    auto data = trace.serialize(SampleFormat::IEEEFloat);

    // Verify size
    size_t expected_size = constants::TRACE_HEADER_SIZE + 10 * sizeof(float);
    assert(data.size() == expected_size);

    // Parse back
    Trace<float> parsed = Trace<float>::parse(data, SampleFormat::IEEEFloat);

    // Verify header
    assert(parsed.header().trace_sequence() == 42);
    assert(parsed.header().inline_number() == 100);
    assert(parsed.header().num_samples() == 10);

    // Verify samples
    auto parsed_samples = parsed.samples();
    for (size_t i = 0; i < 10; ++i) {
        float expected = static_cast<float>(i) * 1.5f;
        assert(std::abs(parsed_samples[i] - expected) < 0.001f);
    }

    std::cout << "✓ Trace serialization (IEEE) tests passed\n";
}

void test_trace_serialization_ibm() {
    // Create trace with simple values
    TraceHeader header;
    header.set_num_samples(5);
    header.set_trace_sequence(1);

    Trace<float> trace(header, 5);
    auto samples = trace.samples();
    samples[0] = 1.0f;
    samples[1] = 2.0f;
    samples[2] = -1.0f;
    samples[3] = 0.5f;
    samples[4] = 100.0f;

    // Serialize as IBM
    auto data = trace.serialize(SampleFormat::IBMFloat);

    // Parse back
    Trace<float> parsed = Trace<float>::parse(data, SampleFormat::IBMFloat);

    // Verify samples (allow more tolerance for IBM conversion)
    auto parsed_samples = parsed.samples();
    assert(std::abs(parsed_samples[0] - 1.0f) < 0.01f);
    assert(std::abs(parsed_samples[1] - 2.0f) < 0.01f);
    assert(std::abs(parsed_samples[2] - (-1.0f)) < 0.01f);
    assert(std::abs(parsed_samples[3] - 0.5f) < 0.01f);
    assert(std::abs(parsed_samples[4] - 100.0f) < 1.0f);

    std::cout << "✓ Trace serialization (IBM) tests passed\n";
}

void test_trace_int_formats() {
    TraceHeader header;
    header.set_num_samples(5);

    // Test Int32
    {
        Trace<int32_t> trace(header, 5);
        auto samples = trace.samples();
        for (size_t i = 0; i < 5; ++i) {
            samples[i] = static_cast<int32_t>(i * 1000);
        }

        auto data = trace.serialize(SampleFormat::Int32);
        auto parsed = Trace<int32_t>::parse(data, SampleFormat::Int32);

        for (size_t i = 0; i < 5; ++i) {
            assert(parsed.samples()[i] == static_cast<int32_t>(i * 1000));
        }
    }

    // Test Int16
    {
        Trace<int16_t> trace(header, 5);
        auto samples = trace.samples();
        for (size_t i = 0; i < 5; ++i) {
            samples[i] = static_cast<int16_t>(i * 100);
        }

        auto data = trace.serialize(SampleFormat::Int16);
        auto parsed = Trace<int16_t>::parse(data, SampleFormat::Int16);

        for (size_t i = 0; i < 5; ++i) {
            assert(parsed.samples()[i] == static_cast<int16_t>(i * 100));
        }
    }

    std::cout << "✓ Trace integer format tests passed\n";
}

int main() {
    std::cout << "Running trace tests...\n";

    test_trace_construction();
    test_trace_serialization_ieee();
    test_trace_serialization_ibm();
    test_trace_int_formats();

    std::cout << "\n🎉 All trace tests passed!\n";
    return 0;
}
