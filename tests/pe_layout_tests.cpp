#include "horizon/pe_layout.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

using horizon::PeLayout;

void write_u16(std::vector<std::uint8_t>& b, std::size_t o, std::uint16_t v) {
    b[o] = static_cast<std::uint8_t>(v & 0xFFU);
    b[o + 1U] = static_cast<std::uint8_t>((v >> 8U) & 0xFFU);
}

void write_u32(std::vector<std::uint8_t>& b, std::size_t o, std::uint32_t v) {
    for (std::size_t i = 0; i < 4U; ++i) {
        b[o + i] = static_cast<std::uint8_t>((v >> (8U * i)) & 0xFFU);
    }
}

void write_u64(std::vector<std::uint8_t>& b, std::size_t o, std::uint64_t v) {
    write_u32(b, o, static_cast<std::uint32_t>(v & 0xFFFFFFFFULL));
    write_u32(b, o + 4U, static_cast<std::uint32_t>(v >> 32U));
}

void write_text(std::vector<std::uint8_t>& b, std::size_t o, const std::string& s) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        b[o + i] = static_cast<std::uint8_t>(s[i]);
    }
}

// Fixed synthetic layout (PE32+):
//   0x080 NT headers, 0x098 optional header (0xF0 bytes), 0x188 section table
//   .text  VA 0x1000, raw 0x400 (0x200 bytes)
//   .idata VA 0x2000, raw 0x600 (0x200 bytes)  -- import directory
constexpr std::size_t kNt = 0x80U;
constexpr std::size_t kFileHeader = kNt + 4U;
constexpr std::size_t kOptional = kFileHeader + 20U;
constexpr std::size_t kSections = kOptional + 0xF0U;
constexpr std::size_t kIdataRaw = 0x600U;

std::vector<std::uint8_t> make_pe(bool pe32_plus = true) {
    std::vector<std::uint8_t> b(0x800U, 0U);
    write_u16(b, 0U, 0x5A4DU);
    write_u32(b, 0x3CU, static_cast<std::uint32_t>(kNt));
    write_text(b, kNt, "PE");

    write_u16(b, kFileHeader, 0x8664U);
    write_u16(b, kFileHeader + 2U, 2U);
    // Optional-header size: PE32+ = 112 + 16*8 = 0xF0. PE32 = 96 + 16*8 = 0xE0.
    write_u16(b, kFileHeader + 16U, pe32_plus ? 0x00F0U : 0x00E0U);

    write_u16(b, kOptional, pe32_plus ? 0x020BU : 0x010BU);
    write_u32(b, kOptional + 16U, 0x1010U);          // entry point RVA
    if (pe32_plus) {
        write_u64(b, kOptional + 24U, 0x140000000ULL);
    } else {
        write_u32(b, kOptional + 28U, 0x00400000U);
    }
    write_u32(b, kOptional + 32U, 0x1000U);           // section alignment
    write_u32(b, kOptional + 36U, 0x200U);            // file alignment
    write_u32(b, kOptional + 56U, 0x3000U);           // size of image
    write_u32(b, kOptional + 60U, 0x400U);            // size of headers
    write_u16(b, kOptional + 68U, 14U);               // subsystem
    const std::size_t count_off = kOptional + (pe32_plus ? 108U : 92U);
    const std::size_t dirs_off = kOptional + (pe32_plus ? 112U : 96U);
    write_u32(b, count_off, 16U);
    write_u32(b, dirs_off + 8U, 0x2000U);             // import directory RVA
    write_u32(b, dirs_off + 12U, 0x3CU);              // 2 descriptors + null terminator
    write_u32(b, dirs_off + 4U * 8U, 0x700U);         // security directory is a file offset
    write_u32(b, dirs_off + 4U * 8U + 4U, 0x40U);

    const std::size_t sections = kOptional + (pe32_plus ? 0xF0U : 0xE0U);
    write_text(b, sections, ".text");
    write_u32(b, sections + 8U, 0x200U);
    write_u32(b, sections + 12U, 0x1000U);
    write_u32(b, sections + 16U, 0x200U);
    write_u32(b, sections + 20U, 0x400U);
    write_u32(b, sections + 36U, 0x60000020U);        // R-X

    write_text(b, sections + 40U, ".idata");
    write_u32(b, sections + 48U, 0x200U);
    write_u32(b, sections + 52U, 0x2000U);
    write_u32(b, sections + 56U, 0x200U);
    write_u32(b, sections + 60U, static_cast<std::uint32_t>(kIdataRaw));
    write_u32(b, sections + 76U, 0xC0000040U);        // RW-

    // Import descriptors at RVA 0x2000 (file 0x600), names at 0x2100 / 0x2110.
    write_u32(b, kIdataRaw + 12U, 0x2100U);
    write_u32(b, kIdataRaw + 20U + 12U, 0x2110U);
    // Third descriptor stays zero: terminator.
    write_text(b, kIdataRaw + 0x100U, "KERNEL32.dll");
    write_text(b, kIdataRaw + 0x110U, "d3d12.dll");
    return b;
}

std::size_t sections_offset(bool pe32_plus = true) {
    return kOptional + (pe32_plus ? 0xF0U : 0xE0U);
}

bool check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }
    return true;
}

bool test_layout_fields_pe32_plus() {
    const auto bytes = make_pe();
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    if (!check(result.ok, "valid PE32+ layout should parse: " + result.error)) {
        return false;
    }
    const PeLayout& l = result.layout;
    return check(l.image_base == 0x140000000ULL, "PE32+ image base is 64-bit") &&
        check(l.entry_point_rva == 0x1010U, "entry point RVA") &&
        check(l.section_alignment == 0x1000U && l.file_alignment == 0x200U, "alignments") &&
        check(l.size_of_image == 0x3000U && l.size_of_headers == 0x400U, "image/header sizes") &&
        check(l.data_directories.size() == 16U, "16 data directories") &&
        check(l.data_directories[1].address_or_offset == 0x2000U, "import directory RVA") &&
        check(l.sections.size() == 2U, "two sections") &&
        check(l.sections[0].name == ".text" && l.sections[1].name == ".idata", "section names") &&
        check(horizon::section_permissions(l.sections[0].characteristics) == "R-X", ".text is R-X") &&
        check(horizon::section_permissions(l.sections[1].characteristics) == "RW-", ".idata is RW-") &&
        check(l.warnings.empty(), "no warnings for a well-formed image");
}

bool test_security_directory_uses_file_offset_field() {
    const auto bytes = make_pe();
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    if (!check(result.ok, "security directory fixture should parse")) {
        return false;
    }
    return check(result.layout.data_directories.size() > horizon::kDirectorySecurity,
                 "security directory entry should be present") &&
        check(result.layout.data_directories[horizon::kDirectorySecurity].address_or_offset == 0x700U,
              "security directory value is preserved as a file offset");
}

bool test_layout_fields_pe32() {
    const auto bytes = make_pe(false);
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    return check(result.ok, "valid PE32 layout should parse: " + result.error) &&
        check(result.layout.image_base == 0x00400000ULL, "PE32 image base") &&
        check(result.layout.data_directories[1].address_or_offset == 0x2000U, "PE32 import directory RVA");
}

bool test_rva_translation() {
    const auto bytes = make_pe();
    const auto layout = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes)).layout;
    const auto size = bytes.size();
    const auto text = horizon::rva_to_file_offset(layout, 0x1010U, 4U, size);
    const auto header = horizon::rva_to_file_offset(layout, 0x80U, 4U, size);
    return check(text && *text == 0x410U, ".text RVA maps to raw offset") &&
        check(header && *header == 0x80U, "header RVA maps 1:1") &&
        check(!horizon::rva_to_file_offset(layout, 0x11FEU, 4U, size), "range crossing section end is rejected") &&
        check(!horizon::rva_to_file_offset(layout, 0x5000U, 1U, size), "unmapped RVA is rejected") &&
        check(!horizon::rva_to_file_offset(layout, 0xFFFFFFFFU, 16U, size), "RVA arithmetic cannot wrap") &&
        check(!horizon::rva_to_file_offset(layout, 0x1010U, static_cast<std::size_t>(-1), size),
              "very large RVA ranges are rejected without arithmetic wraparound") &&
        check(!horizon::rva_to_file_offset(layout, 0x2000U, 1U, 0x601U - 1U), "file shorter than raw data is rejected");
}

bool test_import_dll_names() {
    const auto bytes = make_pe();
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    const auto imports = horizon::list_import_dlls(view, layout);
    return check(imports.ok, "imports should parse: " + imports.error) &&
        check(imports.dll_names.size() == 2U, "two imported DLLs") &&
        check(imports.dll_names.size() == 2U && imports.dll_names[0] == "KERNEL32.dll", "first DLL") &&
        check(imports.dll_names.size() == 2U && imports.dll_names[1] == "d3d12.dll", "second DLL");
}

bool test_no_import_directory_is_empty_success() {
    auto bytes = make_pe();
    write_u32(bytes, kOptional + 112U + 8U, 0U);
    write_u32(bytes, kOptional + 112U + 12U, 0U);
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    const auto imports = horizon::list_import_dlls(view, layout);
    return check(imports.ok && imports.dll_names.empty(), "absent import directory is not an error");
}

bool test_rejects_directory_count_overflow() {
    auto bytes = make_pe();
    write_u32(bytes, kOptional + 108U, 17U);  // 17 entries cannot fit in 0xF0 bytes
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    return check(!result.ok, "directory count larger than the optional header must fail");
}

bool test_rejects_small_optional_header() {
    auto bytes = make_pe();
    write_u16(bytes, kFileHeader + 16U, 0x0060U);  // below PE32+ fixed size
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    return check(!result.ok, "optional header smaller than PE32+ fixed fields must fail");
}

bool test_rejects_truncated_section_table() {
    auto bytes = make_pe();
    bytes.resize(sections_offset() + 60U);  // second section header cut short
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    return check(!result.ok, "truncated section table must fail");
}

bool test_import_directory_size_is_enforced() {
    auto bytes = make_pe();
    // Only the first non-null descriptor is inside this declared size. The
    // second descriptor and the null terminator are physically present later,
    // but must not be read as part of the import directory.
    write_u32(bytes, kOptional + 112U + 12U, 0x14U);
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    const auto imports = horizon::list_import_dlls(view, layout);
    return check(!imports.ok, "parser must not read descriptors beyond directory.size");
}

bool test_import_directory_rva_size_pair_is_validated() {
    auto bytes = make_pe();
    write_u32(bytes, kOptional + 112U + 8U, 0U);
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    if (!check(!horizon::list_import_dlls(view, layout).ok,
               "non-zero import size with zero RVA must fail")) {
        return false;
    }

    auto second = make_pe();
    write_u32(second, kOptional + 112U + 12U, 0U);
    const std::span<const std::uint8_t> second_view(second);
    const auto second_layout = horizon::read_pe_layout(second_view).layout;
    return check(!horizon::list_import_dlls(second_view, second_layout).ok,
                 "non-zero import RVA with zero size must fail");
}

bool test_import_name_without_terminator() {
    auto bytes = make_pe();
    for (std::size_t i = 0; i < 0x100U; ++i) {
        bytes[kIdataRaw + 0x100U + i] = 'A';  // runs to the end of the section
    }
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    return check(!horizon::list_import_dlls(view, layout).ok, "unterminated DLL name must fail");
}

bool test_import_name_rva_out_of_range() {
    auto bytes = make_pe();
    write_u32(bytes, kIdataRaw + 12U, 0x9000U);
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    return check(!horizon::list_import_dlls(view, layout).ok, "DLL name RVA outside sections must fail");
}

bool test_import_name_non_printable() {
    auto bytes = make_pe();
    bytes[kIdataRaw + 0x100U] = 0x07U;
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    return check(!horizon::list_import_dlls(view, layout).ok, "non-printable DLL name must fail");
}

bool test_import_table_without_terminator() {
    auto bytes = make_pe();
    write_u32(bytes, kOptional + 112U + 12U, 0x200U); // Entire declared .idata raw area.
    // Fill the whole .idata raw area with non-zero descriptors pointing at a valid name.
    for (std::size_t d = 0; d + 20U <= 0x200U; d += 20U) {
        write_u32(bytes, kIdataRaw + d + 12U, 0x2100U);
    }
    const std::span<const std::uint8_t> view(bytes);
    const auto layout = horizon::read_pe_layout(view).layout;
    return check(!horizon::list_import_dlls(view, layout).ok,
                 "import table that runs off the section must fail, not loop");
}

bool test_overlap_and_truncation_warnings() {
    auto bytes = make_pe();
    write_u32(bytes, sections_offset() + 40U + 12U, 0x1100U);  // .idata VA overlaps .text
    const auto overlap = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));

    auto shorter = make_pe();
    shorter.resize(0x700U);  // only .idata raw data is cut short
    const auto truncated = horizon::read_pe_layout(std::span<const std::uint8_t>(shorter));

    return check(overlap.ok && overlap.layout.warnings.size() == 1U, "overlap is reported as a warning") &&
        check(truncated.ok && truncated.layout.warnings.size() == 1U, "missing raw data is reported as a warning");
}

bool test_ambiguous_virtual_rva_is_rejected() {
    auto bytes = make_pe();
    // Move .idata so its virtual range overlaps .text.
    write_u32(bytes, sections_offset() + 40U + 12U, 0x1100U);
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    if (!check(result.ok, "overlapping sections remain inspectable")) {
        return false;
    }
    return check(!horizon::rva_to_file_offset(result.layout, 0x1110U, 4U, bytes.size()),
                 "ambiguous RVA must not resolve by section-table order");
}

bool test_overlapping_raw_sections_are_reported() {
    auto bytes = make_pe();
    write_u32(bytes, sections_offset() + 40U + 20U, 0x480U);
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    if (!check(result.ok, "raw-overlap image remains inspectable")) {
        return false;
    }
    for (const std::string& warning : result.layout.warnings) {
        if (warning.find("overlaps the raw file range") != std::string::npos) {
            return true;
        }
    }
    return check(false, "overlapping raw section ranges must be reported");
}

bool test_raw_section_overlapping_headers_is_reported() {
    auto bytes = make_pe();
    write_u32(bytes, sections_offset() + 20U, 0x200U);
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    if (!check(result.ok, "header-overlap image remains inspectable")) {
        return false;
    }
    for (const std::string& warning : result.layout.warnings) {
        if (warning.find("overlaps the declared PE headers") != std::string::npos) {
            return true;
        }
    }
    return check(false, "raw section overlapping SizeOfHeaders must be reported");
}

bool test_section_name_is_sanitised() {
    auto bytes = make_pe();
    bytes[sections_offset()] = 0x01U;
    const auto result = horizon::read_pe_layout(std::span<const std::uint8_t>(bytes));
    return check(result.ok && result.layout.sections[0].name == "?text",
                 "control characters in section names are replaced");
}

} // namespace

int main() {
    struct TestCase {
        const char* name;
        bool (*run)();
    };

    const TestCase tests[] = {
        {"PE32+ layout fields", test_layout_fields_pe32_plus},
        {"security directory file offset", test_security_directory_uses_file_offset_field},
        {"PE32 layout fields", test_layout_fields_pe32},
        {"RVA to file offset", test_rva_translation},
        {"import DLL names", test_import_dll_names},
        {"import directory size is enforced", test_import_directory_size_is_enforced},
        {"import directory RVA/size pair", test_import_directory_rva_size_pair_is_validated},
        {"absent import directory", test_no_import_directory_is_empty_success},
        {"data-directory count overflow", test_rejects_directory_count_overflow},
        {"optional header too small", test_rejects_small_optional_header},
        {"truncated section table", test_rejects_truncated_section_table},
        {"unterminated DLL name", test_import_name_without_terminator},
        {"DLL name RVA out of range", test_import_name_rva_out_of_range},
        {"non-printable DLL name", test_import_name_non_printable},
        {"import table without terminator", test_import_table_without_terminator},
        {"overlap and truncation warnings", test_overlap_and_truncation_warnings},
        {"ambiguous virtual RVA is rejected", test_ambiguous_virtual_rva_is_rejected},
        {"overlapping raw sections are reported", test_overlapping_raw_sections_are_reported},
        {"raw section/header overlap is reported", test_raw_section_overlapping_headers_is_reported},
        {"section name sanitising", test_section_name_is_sanitised},
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
    std::cout << (total - static_cast<std::size_t>(failures)) << "/" << total << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
