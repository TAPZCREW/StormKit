#ifndef CPP_BINDINGS_HPP
#define CPP_BINDINGS_HPP

#include <cstdint>
#include <limits>

using id = std::uint64_t;

extern "C" {
    // enum class _Key : std::uint8_t {
    //     a = 0,
    //     b,
    //     c,
    //     d,
    //     e,
    //     f,
    //     g,
    //     h,
    //     i,
    //     j,
    //     k,
    //     l,
    //     m,
    //     n,
    //     o,
    //     p,
    //     q,
    //     r,
    //     s,
    //     t,
    //     u,
    //     v,
    //     w,
    //     x,
    //     y,
    //     z,
    //     num_0,
    //     num_1,
    //     num_2,
    //     num_3,
    //     num_4,
    //     num_5,
    //     num_6,
    //     num_7,
    //     num_8,
    //     num_9,
    //     escape,
    //     l_control,
    //     l_shift,
    //     l_alt,
    //     l_meta,
    //     r_control,
    //     r_shift,
    //     r_alt,
    //     r_meta,
    //     menu,
    //     l_bracket,
    //     r_bracket,
    //     semi_colon,
    //     comma,
    //     period,
    //     quote,
    //     slash,
    //     back_slash,
    //     tilde,
    //     equal,
    //     hyphen,
    //     space,
    //     enter,
    //     back_space,
    //     tab,
    //     page_up,
    //     page_down,
    //     begin,
    //     end,
    //     home,
    //     insert,
    //     delete,
    //     add,
    //     substract,
    //     multiply,
    //     divide,
    //     left,
    //     right,
    //     up,
    //     down,
    //     numpad_0,
    //     numpad_1,
    //     numpad_2,
    //     numpad_3,
    //     numpad_4,
    //     numpad_5,
    //     numpad_6,
    //     numpad_7,
    //     numpad_8,
    //     numpad_9,
    //     f1,
    //     f2,
    //     f3,
    //     f4,
    //     f5,
    //     f6,
    //     f7,
    //     f8,
    //     f9,
    //     f10,
    //     f11,
    //     f12,
    //     f13,
    //     f14,
    //     f15,
    //     pause,
    //     unknown = std::numeric_limits<std::uint8_t>::max(),
    // };

    auto localizedKey(char code) noexcept -> std::uint8_t;
    auto usageToVirtualCode(std::int32_t usage) noexcept -> std::int32_t;

    auto swiftClosedEvent(id) noexcept -> bool;
    auto swiftResizedEvent(id, float, float) noexcept -> void;
    auto swiftRestoredEvent(id) noexcept -> void;
    auto swiftMinimizedEvent(id) noexcept -> void;
    auto swiftActivatedEvent(id) noexcept -> void;
    auto swiftDeactivatedEvent(id) noexcept -> void;

    auto swiftMouseDownEvent(id, std::int32_t, std::int32_t, std::int32_t) noexcept -> void;
    auto swiftMouseUpEvent(id, std::int32_t, std::int32_t, std::int32_t) noexcept -> void;
    auto swiftMouseMovedEvent(id, std::int32_t, std::int32_t) noexcept -> void;

    auto swiftKeyDownEvent(id, std::uint16_t, char) noexcept -> void;
    auto swiftKeyUpEvent(id, std::uint16_t, char) noexcept -> void;
}

#endif
