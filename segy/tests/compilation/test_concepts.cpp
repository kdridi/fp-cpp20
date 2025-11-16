#include "../../include/segy/concepts/segy_concepts.hpp"

using namespace segy::concepts;

// ============================================================================
// COMPILE-TIME TESTS (static_assert)
// ============================================================================

// Test ValidSampleFormat concept
static_assert(ValidSampleFormat<SampleFormat::IBMFloat>);
static_assert(ValidSampleFormat<SampleFormat::IEEEFloat>);
static_assert(ValidSampleFormat<SampleFormat::Int32>);
static_assert(ValidSampleFormat<SampleFormat::Int16>);
static_assert(ValidSampleFormat<SampleFormat::Int8>);

// Test ByteSwappable concept
static_assert(ByteSwappable<uint8_t>);
static_assert(ByteSwappable<uint16_t>);
static_assert(ByteSwappable<uint32_t>);
static_assert(ByteSwappable<uint64_t>);
static_assert(ByteSwappable<int16_t>);
static_assert(ByteSwappable<int32_t>);

// Test constants
static_assert(constants::TEXTUAL_HEADER_SIZE == 3200);
static_assert(constants::BINARY_HEADER_SIZE == 400);
static_assert(constants::TRACE_HEADER_SIZE == 240);

static_assert(constants::MAX_SAMPLES_PER_TRACE == 32767);
static_assert(constants::MIN_SAMPLE_INTERVAL == 1);
static_assert(constants::MAX_SAMPLE_INTERVAL == 1000000);

// Test FieldMetadata
using TestField = FieldMetadata<100, int32_t>;
static_assert(TestField::offset == 100);
static_assert(TestField::size == 4);

// Ensure this file compiles = all static_asserts pass
int main() {
    return 0;
}
