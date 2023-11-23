#include <fmt/format.h>
#include <fmt/color.h>

#include "window.hpp"

using pooper_cube::window_t;

namespace {
    auto error_callback(int error_code, const char* description) -> void {
        fmt::print(fmt::fg(fmt::color::red), "[GLFW ERROR {}]: {}\n", error_code, description);
    }
}

window_t::window_t(uint16_t p_width, uint16_t p_height, std::string_view p_title) {
    if (!glfwInit()) {
        throw creation_exception_t::glfw_init_failed;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_window = glfwCreateWindow(p_width, p_height, p_title.data(), nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw creation_exception_t::window_creation_failed;
    }
}
