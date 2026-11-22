#include "horizon/pe_inspector.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kHeaderScanLimit = 1024U * 1024U;

void print_usage(const char* executable) {
    std::cerr << "Usage: " << executable << " <authorised-pe-file>\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string path = argv[1];
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        std::cerr << "Error: unable to open input file.\n";
        return 3;
    }

    const std::streamoff file_size = input.tellg();
    if (file_size < 0) {
        std::cerr << "Error: unable to determine input size.\n";
        return 3;
    }

    const auto scan_size = static_cast<std::size_t>(
        std::min<std::streamoff>(
            file_size,
            static_cast<std::streamoff>(kHeaderScanLimit)));

    std::vector<std::uint8_t> header_bytes(scan_size);
    input.seekg(0, std::ios::beg);

    if (!header_bytes.empty()) {
        input.read(
            reinterpret_cast<char*>(header_bytes.data()),
            static_cast<std::streamsize>(header_bytes.size()));
        if (!input) {
            std::cerr << "Error: unable to read the file header.\n";
            return 3;
        }
    }

    const horizon::PeParseResult parsed =
        horizon::inspect_pe(std::span<const std::uint8_t>(header_bytes));

    if (!parsed.ok) {
        std::cerr << "PE inspection failed: " << parsed.error << '\n';
        return 4;
    }

    const auto& metadata = parsed.metadata;

    std::cout << "HORIZON — read-only PE inspector\n";
    std::cout << "File: " << path << '\n';
    std::cout << "File size: " << file_size << " bytes\n";
    std::cout << "Machine: " << horizon::machine_name(metadata.machine)
              << " (0x" << std::hex << std::uppercase
              << std::setw(4) << std::setfill('0') << metadata.machine << ")\n";
    std::cout << "Optional header: "
              << horizon::optional_header_name(metadata.optional_header_magic) << '\n';
    std::cout << "Subsystem: " << horizon::subsystem_name(metadata.subsystem)
              << " (" << std::dec << metadata.subsystem << ")\n";
    std::cout << "Sections: " << metadata.section_count << '\n';
    std::cout << "PE header offset: 0x" << std::hex << std::uppercase
              << metadata.nt_header_offset << std::dec << '\n';
    std::cout << "\nThis is metadata inspection only. No code was executed.\n";
    std::cout << "An Xbox subsystem marker does not establish runtime compatibility.\n";

    return 0;
}
