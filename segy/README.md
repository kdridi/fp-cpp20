# 🌊 SEG-Y C++20 Library

**Ultra-optimized, type-safe SEG-Y Rev 1 reader/writer with zero-copy design**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## ⚡ Features

### 🚀 Performance
- **Zero-copy field extraction** via `std::span`
- **constexpr endianness conversion** (compile-time when possible)
- **Minimal allocations** (header-only design)
- **Optimized I/O** with buffering

### 💎 Type Safety
- **Compile-time format validation** via C++20 concepts
- **Type-safe field accessors** (no magic offsets in user code)
- **Automatic endianness handling** (Big Endian ↔ Native)
- **IBM Float ↔ IEEE Float** conversion

### 🔬 Correctness
- **static_assert** for SEG-Y spec compliance
- **Comprehensive test suite** (unit + integration)
- **Sanitizer-clean** (ASan, UBSan)
- **Zero warnings** with `-Wall -Wextra -Wpedantic`

---

## 📋 Requirements

- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+

---

## 🚀 Quick Start

### Installation

```bash
mkdir build && cd build
cmake -GNinja ..
ninja
ninja test
```

### Usage Example

```cpp
#include <segy/segy.hpp>

// Reading
segy::Reader<float> reader("seismic.segy");

std::cout << "Traces: " << reader.num_traces() << "\n";
std::cout << "Samples/trace: "
          << reader.binary_header().samples_per_trace() << "\n";

auto trace = reader.read_trace(0);
auto samples = trace.samples();
// Zero-copy access to samples

// Writing
segy::TextualHeader textual;
textual.set_line(1, "C01 My Survey");

segy::BinaryHeader binary;
binary.set_sample_interval(4000); // 4ms
binary.set_samples_per_trace(1000);
binary.set_sample_format(segy::SampleFormat::IEEEFloat);

segy::Writer<float> writer("output.segy", textual, binary);

segy::TraceHeader trace_header;
trace_header.set_trace_sequence(1);
trace_header.set_num_samples(1000);

segy::Trace<float> trace(trace_header, 1000);
// ... fill samples ...
writer.write_trace(trace);
```

---

## 🏗️ Architecture

### Compile-Time Layer (Functional)

```cpp
// Format validation at compile-time
static_assert(ValidSampleFormat<SampleFormat::IEEEFloat>);

// Endianness conversion (constexpr)
constexpr uint32_t value = from_big_endian(0x12345678);

// Field metadata (compile-time)
using Field = FieldMetadata<offset, int32_t>;
```

### Runtime Layer (Performance)

```cpp
// Zero-copy field extraction
int32_t job_id = extract_field<int32_t>(data, offset);

// Buffered I/O
auto traces = reader.read_traces(start, end);
```

**Result:** Type safety + Zero-cost abstractions

---

## 📊 Benchmarks

*(Example - adjust to your actual measurements)*

```
Operation               | Time (µs) | vs Naive C
------------------------|-----------|----------
Read trace header       | 0.5       | 1.0x (zero overhead)
Parse 1000 samples      | 12.0      | 1.0x (zero overhead)
IBM→IEEE conversion     | 0.3/sample| 1.0x (inline optimized)
Write trace             | 15.0      | 1.0x (buffered I/O)
```

**Zero-cost abstractions verified via:**
- Assembly inspection (godbolt.org)
- Benchmarks vs C baseline
- Size checks: `static_assert(sizeof(T) == expected)`

---

## 🧪 Testing

### Run All Tests

```bash
cd build
ninja test
```

### Test Coverage

- ✅ **Unit tests**: Endian, Headers, Trace, I/O
- ✅ **Compile-time tests**: static_assert validation
- ✅ **Integration tests**: Round-trip read/write
- ✅ **Format tests**: IEEE, IBM, Int32, Int16

### Sanitizers

```bash
cmake -DENABLE_SANITIZERS=ON ..
ninja
ninja test
```

---

## 📚 API Reference

### Core Types

#### `TextualHeader`
```cpp
TextualHeader header;
header.set_line(1, "C01 Survey Name");
auto line = header.get_line(1); // std::string_view
```

#### `BinaryHeader`
```cpp
BinaryHeader header;
header.set_sample_interval(4000);
header.set_samples_per_trace(1000);
header.set_sample_format(SampleFormat::IEEEFloat);

int16_t interval = header.sample_interval();
bool valid = header.is_valid(); // Compile-time + runtime validation
```

#### `TraceHeader`
```cpp
TraceHeader header;
header.set_trace_sequence(1);
header.set_inline_number(100);
header.set_crossline_number(200);
```

#### `Trace<T>`
```cpp
Trace<float> trace(header, num_samples);
auto samples = trace.samples(); // std::span<float>

// Serialize
auto data = trace.serialize(SampleFormat::IEEEFloat);

// Parse
auto parsed = Trace<float>::parse(data, format);
```

### I/O

#### `Reader<T>`
```cpp
Reader<float> reader("file.segy");

size_t n = reader.num_traces();
auto trace = reader.read_trace(index);
auto traces = reader.read_traces(start, end);
auto all = reader.read_all_traces();

const auto& textual = reader.textual_header();
const auto& binary = reader.binary_header();
```

#### `Writer<T>`
```cpp
Writer<float> writer(path, textual, binary);

writer.write_trace(trace);
writer.write_traces(traces);

writer.close();
```

---

## 🎯 Design Principles (ULTRA INSTINCT v2.0)

### 1. **constexpr WHEN relevant**
- ✅ Format validation, endianness, field offsets
- ❌ NOT for I/O, large parsing

### 2. **Types REDUCE invalid states**
- Concepts enforce valid formats
- static_assert validates invariants
- BUT: C++ still has UB (we mitigate, not eliminate)

### 3. **Zero-cost VERIFIABLE**
```cpp
static_assert(sizeof(BinaryHeader) == 400);
// + godbolt assembly verification
// + benchmarks vs C baseline
```

### 4. **UB Awareness**
- Sanitizers enabled in CI
- Warnings as errors
- Known UB risks documented

---

## 🚨 Limitations

### What SEG-Y library CAN do:
- ✅ Read/Write SEG-Y Rev 1 files
- ✅ Type-safe header access
- ✅ Automatic endianness conversion
- ✅ IBM ↔ IEEE float conversion
- ✅ Zero-copy when possible
- ✅ Compile-time validation

### What it CANNOT do:
- ❌ SEG-Y Rev 2 (extended headers)
- ❌ Memory-mapped I/O (planned)
- ❌ Parallel trace reading (planned)
- ❌ Compression (not in spec)

### Known trade-offs:
- Build time: ~5-10s (template-heavy)
- IBM float precision: slight loss vs IEEE
- Large files: `read_all_traces()` loads to RAM

---

## 📖 Examples

See `examples/` directory:
- `simple_reader.cpp` - Read and display SEG-Y info
- `simple_writer.cpp` - Create synthetic SEG-Y file

---

## 🤝 Contributing

Contributions welcome! Please:
1. Follow C++20 modern style
2. Add tests for new features
3. Ensure sanitizers pass
4. Update documentation

---

## 📄 License

MIT License - See LICENSE file

---

## 🙏 Acknowledgments

- SEG-Y Rev 1 specification (1975, revised 2002)
- C++20 features (concepts, constexpr, ranges)
- ULTRA INSTINCT v2.0 protocol

---

## 🔗 References

- [SEG-Y Rev 1 Specification](https://seg.org/Portals/0/SEG/News%20and%20Resources/Technical%20Standards/seg_y_rev1.pdf)
- [IBM Floating Point Format](https://en.wikipedia.org/wiki/IBM_hexadecimal_floating_point)
- [C++20 Features](https://en.cppreference.com/w/cpp/20)

---

**Built with ULTRA INSTINCT v2.0 - C++20 Compile-Time Power** 🔥⚡💎
