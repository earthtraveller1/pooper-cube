#pragma once

#include <cstdint>
#include <string_view>

#include <GLFW/glfw3.h>

namespace pooper_cube {
    // A thin wrapper around a GLFW window, specifically made for this project
    class window_t {
        public:
            window_t(uint16_t width, uint16_t height, std::string_view title);

            window_t(window_t&) = delete;
            auto operator=(window_t&) -> window_t& = delete;

            struct dimensions_t {
                int width;
                int height;
            };

            enum class creation_exception_t {
                glfw_init_failed,
                window_creation_failed,
            };

            // Obtain the width and height of the window.
            auto get_dimensions() const noexcept -> dimensions_t {
                dimensions_t dimensions;
                glfwGetWindowSize(m_window, &dimensions.width, &dimensions.height);
                return dimensions;
            }

            // Show the window.
            auto show() const noexcept -> void {
                glfwShowWindow(m_window);
            }

            // Returns whether the window should close or not.
            auto should_close() const noexcept -> bool {
                return glfwWindowShouldClose(m_window);
            }

            // Poll all the events.
            auto poll_events() const noexcept -> void {
                glfwPollEvents();
            }

            ~window_t() {
                glfwDestroyWindow(m_window);
                glfwTerminate();
            }

        private:
            GLFWwindow* m_window;
    };
}
