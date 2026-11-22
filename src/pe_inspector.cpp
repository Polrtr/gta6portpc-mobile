#include "horizon/pe_inspector.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace horizon {
namespace {

[[nodiscard]] bool has_range(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::size_t length) noexcept {
    return offset <= bytes.size() && length <= bytes.size() - offset;
}

[[nodiscard]] bool read_u16(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::uint16_t& value) noexcept {
    if (!has_range(bytes, offset, 2U)) {
        return false;
    }

    value = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[offset]) |
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U));
    return true;
}

[[nodiscard]] bool read_u32(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::uint32_t& value) noexcept {
    if (!has_range(bytes, offset, 4U)) {
        return false;
    }

    value = static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
    return true;
}

[[nodiscard]] PeParseResult failure(std::string message) {
    PeParseResult result;
    result.error = std::move(message);
    return result;
}

} // namespace

PeParseResult inspect_pe(std::span<const std::uint8_t> image) {
    if (!has_range(image, 0U, 0x40U)) {
        return failure("File is too small to contain a DOS header.");
    }

    std::uint16_t dos_magic = 0;
    if (!read_u16(image, 0U, dos_magic) || dos_magic != 0x5A4DU) {
        return failure("DOS MZ signature not found; this is not a recognised PE image.");
    }

    std::uint32_t nt_offset_u32 = 0;
    if (!read_u32(image, 0x3CU, nt_offset_u32)) {
        return failure("Unable to read the PE header offset.");
    }

    const std::size_t nt_offset = static_cast<std::size_t>(nt_offset_u32);
    if (!has_range(image, nt_offset, 4U + 20U)) {
        return failure("PE header or COFF file header is truncated.");
    }

    if (image[nt_offset] != 0x50U ||
        image[nt_offset + 1U] != 0x45U ||
        image[nt_offset + 2U] != 0x00U ||
        image[nt_offset + 3U] != 0x00U) {
        return failure("PE signature not found at the DOS header offset.");
    }

    const std::size_t file_header = nt_offset + 4U;
    std::uint16_t machine = 0;
    std::uint16_t section_count = 0;
    std::uint16_t optional_header_size = 0;

    if (!read_u16(image, file_header, machine) ||
        !read_u16(image, file_header + 2U, section_count) ||
        !read_u16(image, file_header + 16U, optional_header_size)) {
        return failure("COFF file header is incomplete.");
    }

    const std::size_t optional_header = file_header + 20U;
    if (optional_header_size < 70U ||
        !has_range(image, optional_header, static_cast<std::size_t>(optional_header_size))) {
        return failure("Optional header is truncated or too small to contain the subsystem field.");
    }

    std::uint16_t magic = 0;
    std::uint16_t subsystem = 0;
    if (!read_u16(image, optional_header, magic) ||
        !read_u16(image, optional_header + 68U, subsystem)) {
        return failure("Unable to read optional-header metadata.");
    }

    if (magic != 0x10BU && magic != 0x20BU) {
        return failure("Unsupported optional-header magic; expected PE32 or PE32+.");
    }

    const std::size_t section_table = optional_header +
        static_cast<std::size_t>(optional_header_size);
    const std::size_t section_table_size =
        static_cast<std::size_t>(section_count) * 40U;

    if (!has_range(image, section_table, section_table_size)) {
        return failure("Section-header table is truncated.");
    }

    PeParseResult result;
    result.ok = true;
    result.metadata.machine = machine;
    result.metadata.optional_header_magic = magic;
    result.metadata.subsystem = subsystem;
    result.metadata.section_count = section_count;
    result.metadata.nt_header_offset = nt_offset_u32;
    return result;
}

const char* machine_name(std::uint16_t machine) noexcept {
    switch (machine) {
    case 0x014CU:
        return "I386 (x86)";
    case 0x8664U:
        return "AMD64 (x64)";
    case 0xAA64U:
        return "ARM64";
    case 0xA641U:
        return "ARM64EC";
    default:
        return "Unknown";
    }
}

const char* optional_header_name(std::uint16_t magic) noexcept {
    switch (magic) {
    case 0x10BU:
        return "PE32";
    case 0x20BU:
        return "PE32+";
    default:
        return "Unknown";
    }
}

const char* subsystem_name(std::uint16_t subsystem) noexcept {
    switch (subsystem) {
    case 1U:
        return "Native";
    case 2U:
        return "Windows GUI";
    case 3U:
        return "Windows CUI";
    case 14U:
        return "Xbox";
    default:
        return "Unknown or unsupported";
    }
}

} // namespace horizon
