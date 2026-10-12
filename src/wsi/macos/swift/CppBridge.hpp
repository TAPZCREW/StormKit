#ifndef CPP_BINDINGS_HPP
#define CPP_BINDINGS_HPP

#include <cstdint>

using swift_id = std::uint64_t;

extern "C" {
    auto localizedKey(char code) noexcept -> std::uint8_t;
    auto usageToVirtualCode(std::int32_t usage) noexcept -> std::int32_t;

    auto swiftClosedEvent(swift_id) noexcept -> bool;
    auto swiftResizedEvent(swift_id, float, float) noexcept -> void;
    auto swiftRestoredEvent(swift_id) noexcept -> void;
    auto swiftMinimizedEvent(swift_id) noexcept -> void;
    auto swiftActivatedEvent(swift_id) noexcept -> void;
    auto swiftDeactivatedEvent(swift_id) noexcept -> void;

    auto swiftMouseDownEvent(swift_id, std::int32_t, std::int32_t, std::int32_t) noexcept -> void;
    auto swiftMouseUpEvent(swift_id, std::int32_t, std::int32_t, std::int32_t) noexcept -> void;
    auto swiftMouseMovedEvent(swift_id, std::int32_t, std::int32_t) noexcept -> void;

    auto swiftKeyDownEvent(swift_id, std::uint16_t, char) noexcept -> void;
    auto swiftKeyUpEvent(swift_id, std::uint16_t, char) noexcept -> void;
}

#endif
