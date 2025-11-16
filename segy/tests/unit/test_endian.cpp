#include "../../include/segy/core/endian.hpp"
#include <cassert>
#include <iostream>

using namespace segy::core;

// ============================================================================
// COMPILE-TIME TESTS (static_assert)
// ============================================================================

static_assert(byte_swap<uint16_t>(0x1234) == 0x3412, "byte_swap uint16_t failed");
static_assert(byte_swap<uint32_t>(0x12345678) == 0x78563412, "byte_swap uint32_t failed");
static_assert(byte_swap<uint64_t>(0x0123456789ABCDEF) == 0xEFCDAB8967452301, "byte_swap uint64_t failed");

// ============================================================================
// RUNTIME TESTS
// ============================================================================

void test_byte_swap() {
    // uint16_t
    assert(byte_swap<uint16_t>(0x1234) == 0x3412);
    assert(byte_swap<uint16_t>(0xABCD) == 0xCDAB);

    // uint32_t
    assert(byte_swap<uint32_t>(0x12345678) == 0x78563412);
    assert(byte_swap<uint32_t>(0xDEADBEEF) == 0xEFBEADDE);

    // uint64_t
    assert(byte_swap<uint64_t>(0x0123456789ABCDEF) == 0xEFCDAB8967452301);

    std::cout << "✓ byte_swap tests passed\n";
}

void test_ibm_ieee_conversion() {
    // Test zero
    assert(ibm_to_ieee(0) == 0.0f);
    assert(ieee_to_ibm(0.0f) == 0);

    // Test positive value
    // IBM: 0x41100000 = 1.0 in IBM format
    // (exponent 65-64=1, mantissa 0x100000/0x1000000 = 1/16)
    // 1/16 * 16^1 = 1.0
    float val1 = ibm_to_ieee(0x41100000);
    assert(std::abs(val1 - 1.0f) < 0.001f);

    // Test round-trip
    float original = 3.14159f;
    uint32_t ibm = ieee_to_ibm(original);
    float recovered = ibm_to_ieee(ibm);
    assert(std::abs(original - recovered) < 0.01f); // Some precision loss expected

    std::cout << "✓ IBM <-> IEEE conversion tests passed\n";
}

void test_field_extraction() {
    // Create test data
    std::array<std::byte, 16> data{};

    // Write int32_t at offset 1 (1-indexed)
    int32_t test_value = 0x12345678;
    write_field(std::span(data), 1, test_value);

    // Read it back
    int32_t read_value = extract_field<int32_t>(data, 1);

    // Should match after endianness conversion
    int32_t expected = from_big_endian(test_value);
    assert(read_value == expected);

    std::cout << "✓ field extraction tests passed\n";
}

int main() {
    std::cout << "Running endian tests...\n";

    test_byte_swap();
    test_ibm_ieee_conversion();
    test_field_extraction();

    std::cout << "\n🎉 All endian tests passed!\n";
    return 0;
}
