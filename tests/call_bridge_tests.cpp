#include "horizon/call_bridge.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using horizon::CallBridge;
using horizon::CallRequest;
using horizon::CallResponse;
using horizon::CallStatus;
using horizon::ServiceId;

bool check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }
    return true;
}

bool test_registered_service_dispatch() {
    CallBridge bridge;
    const auto graphics_id = static_cast<std::uint32_t>(ServiceId::Graphics);

    const bool registered = bridge.register_handler(
        graphics_id,
        [](const CallRequest& request) {
            return CallResponse{
                CallStatus::Success,
                {static_cast<std::uint8_t>(request.operation_id)},
                "synthetic graphics handler"
            };
        });

    const auto response = bridge.dispatch(CallRequest{graphics_id, 7U, 1U, {}});
    return check(registered, "graphics handler should register") &&
        check(response.status == CallStatus::Success, "registered handler should run") &&
        check(response.payload.size() == 1U && response.payload[0] == 7U,
              "handler response payload should be returned");
}

bool test_unknown_service_is_rejected() {
    CallBridge bridge;
    const auto response = bridge.dispatch(CallRequest{99U, 1U, 1U, {}});
    return check(response.status == CallStatus::UnsupportedService,
                 "unknown service should return unsupported-service");
}

bool test_invalid_request_is_rejected() {
    CallBridge bridge;
    const auto response = bridge.dispatch(CallRequest{0U, 1U, 1U, {}});
    return check(response.status == CallStatus::InvalidRequest,
                 "zero service ID should be rejected");
}

bool test_duplicate_registration_is_rejected() {
    CallBridge bridge;
    const auto service_id = static_cast<std::uint32_t>(ServiceId::Input);
    const auto handler = [](const CallRequest&) {
        return CallResponse{CallStatus::Success, {}, "ok"};
    };

    const bool first = bridge.register_handler(service_id, handler);
    const bool second = bridge.register_handler(service_id, handler);
    return check(first, "first registration should succeed") &&
        check(!second, "duplicate registration should fail");
}

bool test_oversized_payload_is_rejected() {
    CallBridge bridge;
    std::vector<std::uint8_t> payload(CallBridge::MaxPayloadBytes + 1U, 0U);
    const CallRequest request{
        static_cast<std::uint32_t>(ServiceId::Runtime),
        1U,
        1U,
        std::span<const std::uint8_t>(payload)
    };
    const auto response = bridge.dispatch(request);
    return check(response.status == CallStatus::PayloadTooLarge,
                 "oversized payload should be rejected before dispatch");
}

bool test_oversized_response_is_rejected() {
    CallBridge bridge;
    const auto service_id = static_cast<std::uint32_t>(ServiceId::Graphics);
    const bool registered = bridge.register_handler(
        service_id,
        [](const CallRequest&) {
            return CallResponse{
                CallStatus::Success,
                std::vector<std::uint8_t>(CallBridge::MaxResponseBytes + 1U, 0U),
                "synthetic oversized response"
            };
        });
    const auto response = bridge.dispatch(CallRequest{service_id, 1U, 1U, {}});
    return check(registered, "response handler should register") &&
        check(response.status == CallStatus::ResponseTooLarge,
              "oversized handler response should be rejected") &&
        check(response.payload.empty(), "oversized response payload must not be returned") &&
        check(std::string(horizon::call_status_name(response.status)) == "response-too-large",
              "response-too-large status should have a stable name");
}

bool test_response_detail_is_bounded() {
    CallBridge bridge;
    const auto service_id = static_cast<std::uint32_t>(ServiceId::Audio);
    const bool registered = bridge.register_handler(
        service_id,
        [](const CallRequest&) {
            return CallResponse{
                CallStatus::Success, {},
                std::string(CallBridge::MaxDetailBytes + 128U, 'D')
            };
        });
    const auto response = bridge.dispatch(CallRequest{service_id, 1U, 1U, {}});
    return check(registered, "detail handler should register") &&
        check(response.status == CallStatus::Success, "bounded detail must not change status") &&
        check(response.detail.size() == CallBridge::MaxDetailBytes,
              "handler detail must be capped");
}

bool test_handler_exception_detail_is_bounded() {
    CallBridge bridge;
    const auto service_id = static_cast<std::uint32_t>(ServiceId::Storage);
    const bool registered = bridge.register_handler(
        service_id,
        [](const CallRequest&) -> CallResponse {
            throw std::runtime_error(std::string(CallBridge::MaxDetailBytes * 2U, 'E'));
        });
    const auto response = bridge.dispatch(CallRequest{service_id, 1U, 1U, {}});
    return check(registered, "long-error handler should register") &&
        check(response.status == CallStatus::HandlerError, "exception should remain a handler error") &&
        check(response.detail.size() == CallBridge::MaxDetailBytes,
              "exception diagnostics must be capped");
}

bool test_handler_exception_is_reported() {
    CallBridge bridge;
    const auto service_id = static_cast<std::uint32_t>(ServiceId::Storage);
    const bool registered = bridge.register_handler(
        service_id,
        [](const CallRequest&) -> CallResponse {
            throw std::runtime_error("synthetic failure");
        });

    const auto response = bridge.dispatch(CallRequest{service_id, 1U, 1U, {}});
    return check(registered, "throwing handler should register") &&
        check(response.status == CallStatus::HandlerError,
              "handler exception should become an error response") &&
        check(response.detail.find("synthetic failure") != std::string::npos,
              "safe diagnostic text should be retained");
}

} // namespace

int main() {
    struct TestCase {
        const char* name;
        bool (*run)();
    };

    const TestCase tests[] = {
        {"registered service dispatch", test_registered_service_dispatch},
        {"unknown service rejection", test_unknown_service_is_rejected},
        {"invalid request rejection", test_invalid_request_is_rejected},
        {"duplicate registration rejection", test_duplicate_registration_is_rejected},
        {"oversized payload rejection", test_oversized_payload_is_rejected},
        {"oversized response rejection", test_oversized_response_is_rejected},
        {"response detail is bounded", test_response_detail_is_bounded},
        {"handler exception detail is bounded", test_handler_exception_detail_is_bounded},
        {"handler exception reporting", test_handler_exception_is_reported}
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
