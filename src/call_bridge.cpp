#include "horizon/call_bridge.hpp"

#include <exception>
#include <utility>

namespace horizon {

bool CallBridge::register_handler(std::uint32_t service_id, Handler handler) {
    if (service_id == 0U || !handler || handlers_.contains(service_id)) {
        return false;
    }

    handlers_.emplace(service_id, std::move(handler));
    return true;
}

CallResponse CallBridge::dispatch(const CallRequest& request) const {
    if (request.service_id == 0U ||
        request.operation_id == 0U ||
        request.api_version == 0U) {
        return {
            CallStatus::InvalidRequest,
            {},
            "Service ID, operation ID and API version must be non-zero."
        };
    }

    if (request.payload.size() > MaxPayloadBytes) {
        return {
            CallStatus::PayloadTooLarge,
            {},
            "Request payload exceeds the HORIZON call-bridge limit."
        };
    }

    const auto handler = handlers_.find(request.service_id);
    if (handler == handlers_.end()) {
        return {
            CallStatus::UnsupportedService,
            {},
            "No host handler is registered for this HORIZON service."
        };
    }

    try {
        CallResponse response = handler->second(request);
        if (response.payload.size() > MaxResponseBytes) {
            return {
                CallStatus::ResponseTooLarge,
                {},
                "Host service response exceeds the HORIZON call-bridge limit."
            };
        }
        if (response.detail.size() > MaxDetailBytes) {
            response.detail.resize(MaxDetailBytes);
        }
        return response;
    } catch (const std::exception& error) {
        return {
            CallStatus::HandlerError,
            {},
            std::string("Host service handler threw an exception: ") + error.what()
        };
    } catch (...) {
        return {
            CallStatus::HandlerError,
            {},
            "Host service handler threw an unknown exception."
        };
    }
}

const char* call_status_name(CallStatus status) noexcept {
    switch (status) {
    case CallStatus::Success:
        return "success";
    case CallStatus::InvalidRequest:
        return "invalid-request";
    case CallStatus::PayloadTooLarge:
        return "payload-too-large";
    case CallStatus::ResponseTooLarge:
        return "response-too-large";
    case CallStatus::UnsupportedService:
        return "unsupported-service";
    case CallStatus::HandlerError:
        return "handler-error";
    default:
        return "unknown-status";
    }
}

} // namespace horizon
