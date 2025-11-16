#include "../include/segy/segy.hpp"
#include <iostream>
#include <iomanip>

/// @brief Simple SEG-Y reader example
/// Usage: ./simple_reader <file.segy>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.segy>\n";
        return 1;
    }

    try {
        // Open file
        segy::Reader<float> reader(argv[1]);

        std::cout << "📊 SEG-Y File Information\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

        // Display textual header
        std::cout << "📝 Textual Header (first 3 lines):\n";
        for (size_t i = 1; i <= 3; ++i) {
            std::cout << "  " << reader.textual_header().get_line(i) << "\n";
        }
        std::cout << "\n";

        // Display binary header info
        const auto& binary = reader.binary_header();
        std::cout << "🔧 Binary Header:\n";
        std::cout << "  Job ID:           " << binary.job_id() << "\n";
        std::cout << "  Line Number:      " << binary.line_number() << "\n";
        std::cout << "  Sample Interval:  " << binary.sample_interval() << " µs\n";
        std::cout << "  Samples/Trace:    " << binary.samples_per_trace() << "\n";

        std::cout << "  Sample Format:    ";
        switch (binary.sample_format()) {
            case segy::SampleFormat::IBMFloat:
                std::cout << "IBM Float (32-bit)\n";
                break;
            case segy::SampleFormat::IEEEFloat:
                std::cout << "IEEE Float (32-bit)\n";
                break;
            case segy::SampleFormat::Int32:
                std::cout << "Int32\n";
                break;
            case segy::SampleFormat::Int16:
                std::cout << "Int16\n";
                break;
            case segy::SampleFormat::Int8:
                std::cout << "Int8\n";
                break;
            default:
                std::cout << "Unknown (" << static_cast<int>(binary.sample_format()) << ")\n";
        }
        std::cout << "\n";

        // Display trace info
        std::cout << "📈 Trace Information:\n";
        std::cout << "  Total Traces:     " << reader.num_traces() << "\n";
        std::cout << "\n";

        // Read first trace
        if (reader.num_traces() > 0) {
            auto trace = reader.read_trace(0);

            std::cout << "🔍 First Trace Details:\n";
            std::cout << "  Trace Sequence:   " << trace.header().trace_sequence() << "\n";
            std::cout << "  Inline Number:    " << trace.header().inline_number() << "\n";
            std::cout << "  Crossline Number: " << trace.header().crossline_number() << "\n";
            std::cout << "  Num Samples:      " << trace.num_samples() << "\n";
            std::cout << "\n";

            // Display first 10 samples
            std::cout << "📊 First 10 samples:\n";
            auto samples = trace.samples();
            for (size_t i = 0; i < std::min<size_t>(10, samples.size()); ++i) {
                std::cout << "  [" << std::setw(4) << i << "] = "
                         << std::setw(12) << std::fixed << std::setprecision(6)
                         << samples[i] << "\n";
            }
        }

        std::cout << "\n✅ File read successfully!\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
