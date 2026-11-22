#include "horizon/pe_inspector.hpp"
#include "horizon/pe_layout.hpp"

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
// --layout needs section and import data, which can sit beyond the header
// scan window. Bound the read so a hostile file cannot exhaust memory.
constexpr std::size_t kLayoutReadLimit = 256U * 1024U * 1024U;

void print_usage(const char* executable) {
    std::cerr << "Usage: " << executable << " [--layout] <authorised-pe-file>\n"
              << "  --layout  also print sections, data directories and imported DLL names\n";
}

} // namespace

int main(int argc, char** argv) {
    bool layout_mode = false;
    if (argc == 3 && std::string(argv[1]) == "--layout") {
        layout_mode = true;
    } else if (argc != 2) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string path = argv[argc - 1];
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

    const std::size_t read_limit = layout_mode ? kLayoutReadLimit : kHeaderScanLimit;
    if (layout_mode && static_cast<std::uintmax_t>(file_size) > read_limit) {
        std::cerr << "Error: file exceeds the 256 MiB --layout read limit.\n";
        return 3;
    }

    const auto scan_size = static_cast<std::size_t>(
        std::min<std::streamoff>(
            file_size,
            static_cast<std::streamoff>(read_limit)));

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
    if (layout_mode) {
        const std::span<const std::uint8_t> image(header_bytes);
        const horizon::PeLayoutResult layout_result = horizon::read_pe_layout(image);
        if (!layout_result.ok) {
            std::cerr << "PE layout parsing failed: " << layout_result.error << '\n';
            return 4;
        }
        const horizon::PeLayout& layout = layout_result.layout;

        std::cout << std::setfill(' ');  // The header printout above leaves '0' active.
        std::cout << "\nImage base: 0x" << std::hex << std::uppercase << layout.image_base
                  << "\nEntry point RVA: 0x" << layout.entry_point_rva
                  << "\nSize of image: 0x" << layout.size_of_image << std::dec << '\n';

        std::cout << "\nSections:\n";
        for (const auto& section : layout.sections) {
            std::cout << "  " << std::left << std::setw(8) << section.name << std::right
                      << " VA=0x" << std::hex << std::uppercase << section.virtual_address
                      << " VSize=0x" << section.virtual_size
                      << " Raw=0x" << section.raw_offset
                      << " RawSize=0x" << section.raw_size << std::dec
                      << ' ' << horizon::section_permissions(section.characteristics) << '\n';
        }

        std::cout << "\nData directories:\n";
        for (std::size_t i = 0; i < layout.data_directories.size(); ++i) {
            const auto& directory = layout.data_directories[i];
            std::cout << "  [" << std::dec << i << "] "
                      << (i == horizon::kDirectorySecurity ? "FileOffset=0x" : "RVA=0x")
                      << std::hex << std::uppercase << directory.address_or_offset
                      << " Size=0x" << directory.size << std::dec << '\n';
        }

        const horizon::PeImportResult imports = horizon::list_import_dlls(image, layout);
        std::cout << "\nImported DLLs:\n";
        if (!imports.ok) {
            std::cerr << "Import directory parsing failed: " << imports.error << '\n';
            return 5;
        }
        if (imports.dll_names.empty()) {
            std::cout << "  (none declared)\n";
        } else {
            for (const auto& dll : imports.dll_names) {
                std::cout << "  " << dll << '\n';
            }
        }

        for (const auto& warning : layout.warnings) {
            std::cout << "Warning: " << warning << '\n';
        }
    }

    std::cout << "\nThis is metadata inspection only. No code was executed.\n";
    std::cout << "An Xbox subsystem marker does not establish runtime compatibility.\n";

    return 0;
}
