#include <cstdio>

#include "fmt/color.h"

#include "window.hpp"

auto main() -> int {
    using pooper_cube::window_t;
    try {
        const window_t window{800, 600, "Pooper Cube"};

        window.show();
        while (!window.should_close()) {
            window.poll_events();
        }
    } catch (window_t::creation_exception_t exception) {
        using exception_t = window_t::creation_exception_t;

        switch (exception) {
            case exception_t::glfw_init_failed:
                fmt::print(stderr, fmt::fg(fmt::color::red), "[FATAL ERROR]: Failed to initialize GLFW.\n");
                break;
            case exception_t::window_creation_failed:
                fmt::print(stderr, fmt::fg(fmt::color::red), "[FATAL ERROR]: Failed to create the GLFW window.\n");
                break;
        }
    }
}
