#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "horizon/pe_inspector.hpp"

namespace horizon {

// Section characteristic flags (PE/COFF specification, "Section Flags").
inline constexpr std::uint32_t kSectionMemExecute = 0x20000000U;
inline constexpr std::uint32_t kSectionMemRead = 0x40000000U;
inline constexpr std::uint32_t kSectionMemWrite = 0x80000000U;

// Index of the import table in the optional-header data directories.
inline constexpr std::size_t kDirectoryImport = 1U;
inline constexpr std::size_t kDirectorySecurity = 4U;
inline constexpr std::size_t kMaxDataDirectories = 16U;

struct PeSection {
    std::string name;  // At most 8 characters; non-printable bytes become '?'.
    std::uint32_t virtual_size = 0;
    std::uint32_t virtual_address = 0;
    std::uint32_t raw_size = 0;
    std::uint32_t raw_offset = 0;
    std::uint32_t characteristics = 0;
};

struct PeDataDirectory {
    // Most data directories store an RVA here. The security/certificate
    // directory (index 4) is the PE-format exception and stores a file offset.
    std::uint32_t address_or_offset = 0;
    std::uint32_t size = 0;
};

struct PeLayout {
    PeMetadata metadata{};
    std::uint64_t image_base = 0;
    std::uint32_t entry_point_rva = 0;
    std::uint32_t section_alignment = 0;
    std::uint32_t file_alignment = 0;
    std::uint32_t size_of_image = 0;
    std::uint32_t size_of_headers = 0;
    std::vector<PeSection> sections;
    // Only the directories declared by the image (at most 16).
    std::vector<PeDataDirectory> data_directories;
    // Non-fatal observations (for example, out-of-file raw data, overlapping
    // section ranges, or virtual ranges outside the RVA address space).
    std::vector<std::string> warnings;
};

struct PeLayoutResult {
    bool ok = false;
    PeLayout layout{};
    std::string error;
};

struct PeImportResult {
    bool ok = false;
    std::vector<std::string> dll_names;
    std::string error;
};

// Parses the optional header fields, data-directory table and section table.
// Read-only: the image is never mapped, relocated or executed.
[[nodiscard]] PeLayoutResult read_pe_layout(std::span<const std::uint8_t> image);

// Translates an RVA range to a unique file offset using only bytes present
// in `file_size`. Returns nullopt for unmapped, truncated or ambiguous ranges,
 // and on integer overflow.
[[nodiscard]] std::optional<std::size_t> rva_to_file_offset(
    const PeLayout& layout,
    std::uint32_t rva,
    std::size_t length,
    std::size_t file_size) noexcept;

// Lists DLL names referenced by the import directory. Only names are read;
// function names, thunks and binding data are not. The descriptor array is
// bounded by its declared directory size. Malformed or ambiguous pointers fail closed.
[[nodiscard]] PeImportResult list_import_dlls(
    std::span<const std::uint8_t> image,
    const PeLayout& layout);

// "R-X", "RW-", etc.
[[nodiscard]] std::string section_permissions(std::uint32_t characteristics);

} // namespace horizon
