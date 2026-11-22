#include "horizon/pe_inspector.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::vector<std::uint8_t> make_minimal_pe(std::uint16_t subsystem = 14U) {
    constexpr std::size_t nt_offset = 0x80U;
    constexpr std::size_t file_header = nt_offset + 4U;
    constexpr std::size_t optional_header = file_header + 20U;
    constexpr std::uint16_t optional_header_size = 0x00F0U;
    constexpr std::size_t section_table = optional_header + optional_header_size;

    std::vector<std::uint8_t> bytes(0x200U, 0U);
    write_u16(bytes, 0U, 0x5A4DU);
    write_u32(bytes, 0x3CU, static_cast<std::uint32_t>(nt_offset));

    bytes[nt_offset] = 0x50U;
    bytes[nt_offset + 1U] = 0x45U;
    bytes[nt_offset + 2U] = 0x00U;
    bytes[nt_offset + 3U] = 0x00U;

    write_u16(bytes, file_header, 0x8664U);
    write_u16(bytes, file_header + 2U, 1U);
    write_u16(bytes, file_header + 16U, optional_header_size);

    write_u16(bytes, optional_header, 0x020BU);
    write_u16(bytes, optional_header + 68U, subsystem);

    // The synthetic section-header table occupies 40 bytes.
    bytes[section_table] = static_cast<std::uint8_t>('.');
    return bytes;
}

bool check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }
    return true;
}

bool test_valid_xbox_subsystem_marker() {
    const auto bytes = make_minimal_pe(14U);
    const auto result = horizon::inspect_pe(std::span<const std::uint8_t>(bytes));

    return check(result.ok, "valid synthetic PE should parse") &&
        check(result.metadata.machine == 0x8664U, "machine should be AMD64") &&
        check(result.metadata.optional_header_magic == 0x020BU, "optional header should be PE32+") &&
        check(result.metadata.subsystem == 14U, "subsystem field should be retained") &&
        check(result.metadata.section_count == 1U, "section count should be retained") &&
        check(std::string(horizon::subsystem_name(14U)) == "Xbox", "subsystem label should be Xbox");
}

bool test_valid_windows_subsystem() {
    const auto bytes = make_minimal_pe(3U);
    const auto result = horizon::inspect_pe(std::span<const std::uint8_t>(bytes));

    return check(result.ok, "synthetic Windows PE should parse") &&
        check(std::string(horizon::subsystem_name(3U)) == "Windows CUI",
              "Windows CUI subsystem should be labelled");
}

bool test_rejects_missing_mz() {
    std::vector<std::uint8_t> bytes(0x200U, 0U);
    const auto result = horizon::inspect_pe(std::span<const std::uint8_t>(bytes));
    return check(!result.ok, "missing MZ signature should be rejected");
}

bool test_rejects_bad_pe_signature() {
    auto bytes = make_minimal_pe();
    bytes[0x80U] = 0x58U;
    const auto result = horizon::inspect_pe(std::span<const std::uint8_t>(bytes));
    return check(!result.ok, "bad PE signature should be rejected");
}

bool test_rejects_truncated_section_table() {
    auto bytes = make_minimal_pe();
    bytes.resize(0x1A0U);
    const auto result = horizon::inspect_pe(std::span<const std::uint8_t>(bytes));
    return check(!result.ok, "truncated section table should be rejected");
}

} // namespace

int main() {
    struct TestCase {
        const char* name;
        bool (*run)();
    };

    const TestCase tests[] = {
        {"valid Xbox subsystem marker", test_valid_xbox_subsystem_marker},
        {"valid Windows subsystem", test_valid_windows_subsystem},
        {"missing MZ signature", test_rejects_missing_mz},
        {"bad PE signature", test_rejects_bad_pe_signature},
        {"truncated section table", test_rejects_truncated_section_table},
    };

    int failures = 0;
    for (const auto& test : tests) {
        const bool passed = test.run();
        std::cout << (passed ? "PASS: " : "FAIL: ") << test.name << '\n';
        if (!passed) {
            ++failures;
        }
    }

    const std::size_t total = sizeof(tests) / sizeof(tests[0]);
    const std::size_t passed = total - static_cast<std::size_t>(failures);
    std::cout << passed << "/" << total << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
