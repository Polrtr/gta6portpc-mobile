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

    const std::size_t stored_directories =
        std::min<std::size_t>(directory_count, kMaxDataDirectories);
    for (std::size_t i = 0; i < stored_directories; ++i) {
        PeDataDirectory directory;
        const std::size_t entry = optional_header + fixed_size + (i * 8U);
        if (!read_u32(image, entry, directory.rva) ||
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
        layout.sections.push_back(std::move(section));
    }

    // Overlapping virtual ranges make RVA translation ambiguous; report them.
    for (std::size_t a = 0; a < layout.sections.size(); ++a) {
        for (std::size_t b = a + 1U; b < layout.sections.size(); ++b) {
            const auto& first = layout.sections[a];
            const auto& second = layout.sections[b];
            const std::uint64_t first_end =
                static_cast<std::uint64_t>(first.virtual_address) + first.virtual_size;
            const std::uint64_t second_end =
                static_cast<std::uint64_t>(second.virtual_address) + second.virtual_size;
            if (first.virtual_size != 0U && second.virtual_size != 0U &&
                first.virtual_address < second_end && second.virtual_address < first_end) {
                layout.warnings.push_back(
                    "Sections '" + first.name + "' and '" + second.name +
                    "' have overlapping virtual ranges.");
            }
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
    const std::uint64_t rva_end = static_cast<std::uint64_t>(rva) + length;

    // Header region: mapped 1:1 from the start of the file.
    if (rva_end <= layout.size_of_headers && rva_end <= file_size) {
        return static_cast<std::size_t>(rva);
    }

    for (const PeSection& section : layout.sections) {
        // Only bytes that physically exist in the file can be translated.
        const std::uint32_t backed = section.virtual_size == 0U
            ? section.raw_size
            : std::min(section.virtual_size, section.raw_size);
        const std::uint64_t start = section.virtual_address;
        const std::uint64_t end = start + backed;
        if (rva < start || rva_end > end) {
            continue;
        }

        const std::uint64_t file_end = static_cast<std::uint64_t>(section.raw_offset) +
            (rva - section.virtual_address) + length;
        if (file_end > file_size) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(
            static_cast<std::uint64_t>(section.raw_offset) + (rva - section.virtual_address));
    }
    return std::nullopt;
}

PeImportResult list_import_dlls(std::span<const std::uint8_t> image, const PeLayout& layout) {
    PeImportResult result;
    result.ok = true;

    if (layout.data_directories.size() <= kDirectoryImport) {
        return result;  // Image declares no import directory.
    }
    const PeDataDirectory directory = layout.data_directories[kDirectoryImport];
    if (directory.rva == 0U || directory.size == 0U) {
        return result;
    }

    for (std::size_t index = 0;; ++index) {
        if (index >= kMaxImportDescriptors) {
            return import_failure("Import directory exceeds the descriptor limit.");
        }

        const std::uint64_t descriptor_rva =
            static_cast<std::uint64_t>(directory.rva) + (index * kImportDescriptorSize);
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
