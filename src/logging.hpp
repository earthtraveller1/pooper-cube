#pragma once

#include <fmt/format.h>
#include <fmt/color.h>

namespace pooper_cube {
    template <typename string_t, typename... args_t>
    inline auto log_error(const string_t& str, args_t... args) noexcept -> void {
        fmt::println(fmt::fg(fmt::color::red), "[ERROR]: {}", fmt::format(str, args...));
    }

    template <typename string_t, typename... args_t>
    inline auto log_warning(const string_t& str, args_t... args) noexcept -> void {
        fmt::println(fmt::fg(fmt::color::yellow), "[WARNING]: {}", fmt::format(str, args...));
    }

    template <typename string_t, typename... args_t>
    inline auto log_info(const string_t& str, args_t... args) noexcept -> void {
        fmt::println("[INFO]: {}", fmt::format(str, args...));
    }
    template <typename string_t, typename... args_t>
    inline auto log_verbose(const string_t& str, args_t... args) noexcept -> void {
        fmt::println(fmt::fg(fmt::color::gray), "[VERBOSE]: {}", fmt::format(str, args...));
    }
}

