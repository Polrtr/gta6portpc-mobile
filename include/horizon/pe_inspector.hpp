#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace horizon {

struct PeMetadata {
    std::uint16_t machine = 0;
    std::uint16_t optional_header_magic = 0;
    std::uint16_t subsystem = 0;
    std::uint16_t section_count = 0;
    std::uint32_t nt_header_offset = 0;
};

struct PeParseResult {
    bool ok = false;
    PeMetadata metadata{};
    std::string error;
};

// Reads PE headers only. It never maps or executes the inspected image.
[[nodiscard]] PeParseResult inspect_pe(std::span<const std::uint8_t> image);

[[nodiscard]] const char* machine_name(std::uint16_t machine) noexcept;
[[nodiscard]] const char* optional_header_name(std::uint16_t magic) noexcept;
[[nodiscard]] const char* subsystem_name(std::uint16_t subsystem) noexcept;

} // namespace horizon
