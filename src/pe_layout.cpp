#include "horizon/pe_layout.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace horizon {
namespace {

constexpr std::size_t kSectionHeaderSize = 40U;
constexpr std::size_t kImportDescriptorSize = 20U;
constexpr std::size_t kMaxImportDescriptors = 4096U;
constexpr std::size_t kMaxDllNameBytes = 256U;
constexpr std::size_t kPe32FixedSize = 96U;    // Up to the data directories.
constexpr std::size_t kPe32PlusFixedSize = 112U;

[[nodiscard]] bool has_range(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::size_t length) noexcept {
    return offset <= bytes.size() && length <= bytes.size() - offset;
}

[[nodiscard]] bool read_u16(
    std::span<const std::uint8_t> bytes, std::size_t offset, std::uint16_t& value) noexcept {
    if (!has_range(bytes, offset, 2U)) {
        return false;
    }
    value = static_cast<std::uint16_t>(
        bytes[offset] | (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U));
    return true;
}

[[nodiscard]] bool read_u32(
    std::span<const std::uint8_t> bytes, std::size_t offset, std::uint32_t& value) noexcept {
    if (!has_range(bytes, offset, 4U)) {
        return false;
    }
    value = static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
    return true;
}

[[nodiscard]] bool read_u64(
    std::span<const std::uint8_t> bytes, std::size_t offset, std::uint64_t& value) noexcept {
    std::uint32_t low = 0;
    std::uint32_t high = 0;
    if (!read_u32(bytes, offset, low) || !read_u32(bytes, offset + 4U, high)) {
        return false;
    }
    value = static_cast<std::uint64_t>(low) | (static_cast<std::uint64_t>(high) << 32U);
    return true;
}

[[nodiscard]] PeLayoutResult layout_failure(std::string message) {
    PeLayoutResult result;
    result.error = std::move(message);
    return result;
}

[[nodiscard]] PeImportResult import_failure(std::string message) {
    PeImportResult result;
    result.error = std::move(message);
    return result;
}

[[nodiscard]] std::string sanitise_section_name(std::span<const std::uint8_t> raw) {
    std::string name;
    for (const std::uint8_t byte : raw) {
        if (byte == 0U) {
            break;
        }
        name.push_back((byte >= 0x20U && byte <= 0x7EU) ? static_cast<char>(byte) : '?');
    }
    return name;
}

} // namespace

PeLayoutResult read_pe_layout(std::span<const std::uint8_t> image) {
    // Reuse the existing validated header walk so both parsers agree on
    // what a well-formed PE looks like.
    const PeParseResult base = inspect_pe(image);
    if (!base.ok) {
        return layout_failure(base.error);
    }

    PeLayout layout;
    layout.metadata = base.metadata;

    const std::size_t file_header = static_cast<std::size_t>(base.metadata.nt_header_offset) + 4U;
    const std::size_t optional_header = file_header + 20U;

    std::uint16_t optional_size = 0;
    if (!read_u16(image, file_header + 16U, optional_size)) {
        return layout_failure("COFF file header is incomplete.");
    }

    const bool is_pe32_plus = base.metadata.optional_header_magic == 0x20BU;
    const std::size_t fixed_size = is_pe32_plus ? kPe32PlusFixedSize : kPe32FixedSize;
    if (optional_size < fixed_size) {
        return layout_failure("Optional header is too small for the declared PE format.");
    }

    std::uint32_t directory_count = 0;
    const std::size_t count_offset = optional_header + (is_pe32_plus ? 108U : 92U);
    bool fields_ok =
        read_u32(image, optional_header + 16U, layout.entry_point_rva) &&
        read_u32(image, optional_header + 32U, layout.section_alignment) &&
        read_u32(image, optional_header + 36U, layout.file_alignment) &&
        read_u32(image, optional_header + 56U, layout.size_of_image) &&
        read_u32(image, optional_header + 60U, layout.size_of_headers) &&
        read_u32(image, count_offset, directory_count);

    if (is_pe32_plus) {
        fields_ok = fields_ok && read_u64(image, optional_header + 24U, layout.image_base);
    } else {
        std::uint32_t base32 = 0;
        fields_ok = fields_ok && read_u32(image, optional_header + 28U, base32);
        layout.image_base = base32;
    }
    if (!fields_ok) {
        return layout_failure("Unable to read optional-header fields.");
    }

    const std::size_t available_directories = (optional_size - fixed_size) / 8U;
    if (directory_count > available_directories) {
        return layout_failure("Declared data-directory count does not fit in the optional header.");
    }

    if (directory_count > kMaxDataDirectories) {
        layout.warnings.push_back(
            "Image declares more data directories than HORIZON currently reports; extra entries are ignored.");
    }
    const std::size_t stored_directories =
        std::min<std::size_t>(directory_count, kMaxDataDirectories);
    for (std::size_t i = 0; i < stored_directories; ++i) {
        PeDataDirectory directory;
        const std::size_t entry = optional_header + fixed_size + (i * 8U);
        if (!read_u32(image, entry, directory.address_or_offset) ||
            !read_u32(image, entry + 4U, directory.size)) {
            return layout_failure("Data-directory table is truncated.");
        }
        layout.data_directories.push_back(directory);
    }

    const std::size_t section_table = optional_header + static_cast<std::size_t>(optional_size);
    for (std::size_t i = 0; i < base.metadata.section_count; ++i) {
        const std::size_t entry = section_table + (i * kSectionHeaderSize);
        if (!has_range(image, entry, kSectionHeaderSize)) {
            return layout_failure("Section-header table is truncated.");
        }

        PeSection section;
        section.name = sanitise_section_name(image.subspan(entry, 8U));
        if (!read_u32(image, entry + 8U, section.virtual_size) ||
            !read_u32(image, entry + 12U, section.virtual_address) ||
            !read_u32(image, entry + 16U, section.raw_size) ||
            !read_u32(image, entry + 20U, section.raw_offset) ||
            !read_u32(image, entry + 36U, section.characteristics)) {
            return layout_failure("Section header is incomplete.");
        }

        const std::uint64_t raw_end =
            static_cast<std::uint64_t>(section.raw_offset) + section.raw_size;
        if (section.raw_size != 0U && raw_end > image.size()) {
            layout.warnings.push_back(
                "Section '" + section.name + "' raw data extends beyond the supplied bytes.");
        }
        const std::uint32_t virtual_extent = section.virtual_size == 0U
            ? section.raw_size
            : section.virtual_size;
        const std::uint64_t virtual_end =
            static_cast<std::uint64_t>(section.virtual_address) + virtual_extent;
        if (virtual_extent != 0U && virtual_end > 0x1'0000'0000ULL) {
            layout.warnings.push_back(
                "Section '" + section.name + "' virtual range exceeds the 32-bit RVA address space.");
        }
        layout.sections.push_back(std::move(section));
    }

    // Detect virtual-range overlap in O(n log n), avoiding a quadratic walk
    // over the (16-bit) section count in an untrusted image.
    struct VirtualRange {
        std::uint64_t begin;
        std::uint64_t end;
        const PeSection* section;
    };
    std::vector<VirtualRange> ranges;
    ranges.reserve(layout.sections.size());
    for (const PeSection& section : layout.sections) {
        const std::uint32_t virtual_extent = section.virtual_size == 0U
            ? section.raw_size
            : section.virtual_size;
        if (virtual_extent != 0U) {
            ranges.push_back({
                section.virtual_address,
                static_cast<std::uint64_t>(section.virtual_address) + virtual_extent,
                &section
            });
        }
    }
    std::sort(ranges.begin(), ranges.end(),
        [](const VirtualRange& left, const VirtualRange& right) {
            if (left.begin != right.begin) {
                return left.begin < right.begin;
            }
            return left.end < right.end;
        });

    const VirtualRange* furthest = nullptr;
    for (const VirtualRange& current : ranges) {
        if (furthest != nullptr && current.begin < furthest->end) {
            layout.warnings.push_back(
                "Section '" + current.section->name + "' overlaps the virtual range of section '" +
                furthest->section->name + "'.");
        }
        if (furthest == nullptr || current.end > furthest->end) {
            furthest = &current;
        }
    }

    // Detect raw file-range aliasing and section data that overlaps headers.
    struct RawRange {
        std::uint64_t begin;
        std::uint64_t end;
        const PeSection* section;
    };
    std::vector<RawRange> raw_ranges;
    raw_ranges.reserve(layout.sections.size());
    for (const PeSection& section : layout.sections) {
        if (section.raw_size == 0U) {
            continue;
        }
        const std::uint64_t begin = section.raw_offset;
        const std::uint64_t end = begin + section.raw_size;
        if (begin < layout.size_of_headers && end > 0U) {
            layout.warnings.push_back(
                "Section '" + section.name + "' raw data overlaps the declared PE headers.");
        }
        raw_ranges.push_back({begin, end, &section});
    }
    std::sort(raw_ranges.begin(), raw_ranges.end(),
        [](const RawRange& left, const RawRange& right) {
            if (left.begin != right.begin) {
                return left.begin < right.begin;
            }
            return left.end < right.end;
        });

    const RawRange* furthest_raw = nullptr;
    for (const RawRange& current : raw_ranges) {
        if (furthest_raw != nullptr && current.begin < furthest_raw->end) {
            layout.warnings.push_back(
                "Section '" + current.section->name + "' overlaps the raw file range of section '" +
                furthest_raw->section->name + "'.");
        }
        if (furthest_raw == nullptr || current.end > furthest_raw->end) {
            furthest_raw = &current;
        }
    }

    PeLayoutResult result;
    result.ok = true;
    result.layout = std::move(layout);
    return result;
}

std::optional<std::size_t> rva_to_file_offset(
    const PeLayout& layout,
    std::uint32_t rva,
    std::size_t length,
    std::size_t file_size) noexcept {
    constexpr std::uint64_t kRvaAddressSpaceSize = 0x1'0000'0000ULL;
    const std::uint64_t range_length = static_cast<std::uint64_t>(length);
    if (static_cast<std::size_t>(range_length) != length ||
        range_length > kRvaAddressSpaceSize - static_cast<std::uint64_t>(rva)) {
        return std::nullopt;
    }
    const std::uint64_t rva_end = static_cast<std::uint64_t>(rva) + range_length;

    // Resolve all matches instead of returning the first. Overlapping
    // sections can make an RVA ambiguous, so fail closed in that case.
    std::optional<std::size_t> resolved_offset;
    const auto record_candidate = [&](std::uint64_t offset) -> bool {
        if (offset > std::numeric_limits<std::size_t>::max() ||
            offset > file_size ||
            length > file_size - static_cast<std::size_t>(offset) ||
            resolved_offset.has_value()) {
            return false;
        }
        resolved_offset = static_cast<std::size_t>(offset);
        return true;
    };

    // Header region: mapped 1:1 from the start of the file.
    if (rva_end <= layout.size_of_headers &&
        !record_candidate(static_cast<std::uint64_t>(rva))) {
        return std::nullopt;
    }

    for (const PeSection& section : layout.sections) {
        // Only bytes physically stored on disk can be translated.
        const std::uint32_t backed = section.virtual_size == 0U
            ? section.raw_size
            : std::min(section.virtual_size, section.raw_size);
        const std::uint64_t start = section.virtual_address;
        const std::uint64_t end = start + backed;
        if (backed == 0U || rva < start || rva_end > end) {
            continue;
        }

        const std::uint64_t offset = static_cast<std::uint64_t>(section.raw_offset) +
            (static_cast<std::uint64_t>(rva) - start);
        if (!record_candidate(offset)) {
            return std::nullopt;
        }
    }
    return resolved_offset;
}

PeImportResult list_import_dlls(std::span<const std::uint8_t> image, const PeLayout& layout) {
    PeImportResult result;
    result.ok = true;

    if (layout.data_directories.size() <= kDirectoryImport) {
        return result;  // Image declares no import directory.
    }
    const PeDataDirectory directory = layout.data_directories[kDirectoryImport];
    const bool has_directory_rva = directory.address_or_offset != 0U;
    const bool has_directory_size = directory.size != 0U;
    if (!has_directory_rva && !has_directory_size) {
        return result;
    }
    if (has_directory_rva != has_directory_size) {
        return import_failure("Import directory must declare both a non-zero RVA and size.");
    }

    constexpr std::uint64_t kRvaAddressSpaceSize = 0x1'0000'0000ULL;
    const std::uint64_t directory_end =
        static_cast<std::uint64_t>(directory.address_or_offset) + directory.size;
    if (directory_end > kRvaAddressSpaceSize) {
        return import_failure("Import directory RVA range overflows.");
    }
    if (directory.size < kImportDescriptorSize) {
        return import_failure("Import directory is too small to contain a terminating descriptor.");
    }

    for (std::size_t index = 0;; ++index) {
        if (index >= kMaxImportDescriptors) {
            return import_failure("Import directory exceeds the descriptor limit.");
        }

        const std::uint64_t descriptor_relative =
            static_cast<std::uint64_t>(index) * kImportDescriptorSize;
        if (descriptor_relative > directory.size ||
            kImportDescriptorSize > static_cast<std::uint64_t>(directory.size) - descriptor_relative) {
            return import_failure("Import directory has no complete terminating descriptor within its declared size.");
        }

        const std::uint64_t descriptor_rva =
            static_cast<std::uint64_t>(directory.address_or_offset) + descriptor_relative;
        if (descriptor_rva > std::numeric_limits<std::uint32_t>::max()) {
            return import_failure("Import descriptor RVA overflows.");
        }
        const auto offset = rva_to_file_offset(
            layout, static_cast<std::uint32_t>(descriptor_rva), kImportDescriptorSize, image.size());
        if (!offset) {
            return import_failure("Import descriptor lies outside the available file data.");
        }

        const auto descriptor = image.subspan(*offset, kImportDescriptorSize);
        if (std::all_of(descriptor.begin(), descriptor.end(),
                        [](std::uint8_t byte) { return byte == 0U; })) {
            return result;  // Terminating null descriptor.
        }

        std::uint32_t name_rva = 0;
        if (!read_u32(descriptor, 12U, name_rva) || name_rva == 0U) {
            return import_failure("Import descriptor has no DLL name.");
        }

        std::string name;
        bool terminated = false;
        for (std::size_t i = 0; i < kMaxDllNameBytes; ++i) {
            const std::uint64_t char_rva = static_cast<std::uint64_t>(name_rva) + i;
            if (char_rva > std::numeric_limits<std::uint32_t>::max()) {
                return import_failure("DLL name RVA overflows.");
            }
            const auto char_offset = rva_to_file_offset(
                layout, static_cast<std::uint32_t>(char_rva), 1U, image.size());
            if (!char_offset) {
                return import_failure("DLL name lies outside the available file data.");
            }
            const std::uint8_t byte = image[*char_offset];
            if (byte == 0U) {
                terminated = true;
                break;
            }
            if (byte < 0x20U || byte > 0x7EU) {
                return import_failure("DLL name contains non-printable characters.");
            }
            name.push_back(static_cast<char>(byte));
        }
        if (!terminated) {
            return import_failure("DLL name is not terminated within the length limit.");
        }
        if (name.empty()) {
            return import_failure("Import descriptor has an empty DLL name.");
        }
        result.dll_names.push_back(std::move(name));
    }
}

std::string section_permissions(std::uint32_t characteristics) {
    std::string text = "---";
    if ((characteristics & kSectionMemRead) != 0U) {
        text[0] = 'R';
    }
    if ((characteristics & kSectionMemWrite) != 0U) {
        text[1] = 'W';
    }
    if ((characteristics & kSectionMemExecute) != 0U) {
        text[2] = 'X';
    }
    return text;
}

} // namespace horizon
