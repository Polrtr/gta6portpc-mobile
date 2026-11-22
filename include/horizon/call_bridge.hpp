#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace horizon {

// HORIZON-owned internal service identifiers. These are not Xbox ABI values.
enum class ServiceId : std::uint32_t {
    Runtime = 1,
    Graphics = 2,
    Audio = 3,
    Input = 4,
    Storage = 5,
    Network = 6
};

enum class CallStatus {
    Success,
    InvalidRequest,
    PayloadTooLarge,
    UnsupportedService,
    HandlerError
};

struct CallRequest {
    std::uint32_t service_id = 0;
    std::uint32_t operation_id = 0;
    std::uint32_t api_version = 0;
    std::span<const std::uint8_t> payload{};
};

struct CallResponse {
    CallStatus status = CallStatus::Success;
    std::vector<std::uint8_t> payload;
    std::string detail;
};

class CallBridge final {
public:
    using Handler = std::function<CallResponse(const CallRequest&)>;
    static constexpr std::size_t MaxPayloadBytes = 16U * 1024U * 1024U;

    // Register handlers during initialisation, before dispatch begins.
    [[nodiscard]] bool register_handler(std::uint32_t service_id, Handler handler);

    // Do not modify the registry concurrently with dispatch calls.
    [[nodiscard]] CallResponse dispatch(const CallRequest& request) const;

private:
    std::unordered_map<std::uint32_t, Handler> handlers_;
};

[[nodiscard]] const char* call_status_name(CallStatus status) noexcept;

} // namespace horizon
