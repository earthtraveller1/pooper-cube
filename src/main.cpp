#include "window.hpp"
#include "logging.hpp"

auto neng(pooper_cube::window_t&& window) {
    (void)window;
}

auto main() -> int {
    using pooper_cube::window_t;
    try {
        const window_t window{800, 600, "Pooper Cube"};

        window.show();
        while (!window.should_close()) {
            window.poll_events();
        }
    } catch (window_t::creation_exception_t exception) {
        using pooper_cube::log_error;
        using exception_t = window_t::creation_exception_t;

        switch (exception) {
            case exception_t::glfw_init_failed:
                log_error("Failed to initialize GLFW.");
                break;
            case exception_t::window_creation_failed:
                log_error("Failed to create the GLFW window.");
                break;
        }
    }
}
