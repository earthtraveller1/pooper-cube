#include "common.hpp"
#include "window.hpp"
#include "vulkan-objects.hpp"

auto main(int p_argc, char** p_argv) -> int {
    using pooper_cube::choose_physical_device;
    using pooper_cube::command_pool_t;
    using pooper_cube::debug_messenger_t;
    using pooper_cube::device_t;
    using pooper_cube::instance_t;
    using pooper_cube::no_adequate_physical_device_exception_t;
    using pooper_cube::shader_module_t;
    using pooper_cube::swapchain_t;
    using pooper_cube::vulkan_creation_exception_t;
    using pooper_cube::window_t;
    using pooper_cube::pipeline_layout_t;
    using pooper_cube::graphics_pipeline_t;
    using pooper_cube::buffer_t;

    bool enable_validation = false;

    const std::vector<const char*> argv(p_argv, p_argv + p_argc);
    for (auto arg : argv) {
        if (std::strcmp(arg, "--enable-validation") == 0) {
            enable_validation = true;
        }
    }

    try {
        const window_t window{800, 600, "Pooper Cube"};
        const instance_t instance{enable_validation};
        std::optional<debug_messenger_t> debug_messenger;
        const auto window_surface = window.create_vulkan_surface(instance);

        if (enable_validation) {
            debug_messenger = debug_messenger_t{instance};
        }

        const auto physical_device = choose_physical_device(instance, window_surface);

        { 
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &device_properties);

            fmt::print(stderr, "[INFO]: Selected the {} graphics card.\n", device_properties.deviceName);
        }

        const device_t logical_device{physical_device};
        const swapchain_t swapchain{window, physical_device, logical_device, window_surface};

        const command_pool_t command_pool{logical_device, physical_device.graphics_queue_family};

        const shader_module_t vertex_shader{logical_device, shader_module_t::type_t::vertex, "shaders/triangle.vert.spv"};
        const shader_module_t fragment_shader{logical_device, shader_module_t::type_t::fragment, "shaders/triangle.frag.spv"};

        const std::vector<VkDescriptorSetLayout> set_layouts;
        const std::vector<VkPushConstantRange> push_constant_ranges;

        const pipeline_layout_t pipeline_layout{logical_device, set_layouts, push_constant_ranges};
        const graphics_pipeline_t graphics_pipeline{logical_device, vertex_shader, fragment_shader, pipeline_layout};

        const buffer_t vertex_buffer{physical_device, logical_device, buffer_t::type_t::vertex, 3*sizeof(float)};

        {
            const pooper_cube::vertex_t vertices[] {
                {{0.0f, -0.5f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}}
            };

            const pooper_cube::staging_buffer_t staging_buffer{physical_device, logical_device, sizeof(vertices)};

            {
                const auto memory = staging_buffer.map_memory();
                std::memcpy(memory, vertices, sizeof(vertices));
            }

            vertex_buffer.copy_from(staging_buffer, command_pool);
        }

        window.show();
        while (!window.should_close()) {
            window.poll_events();
        }
    } catch (window_t::creation_exception_t exception) {
        using exception_t = window_t::creation_exception_t;

        switch (exception) {
            case exception_t::glfw_init_failed:
                fmt::print(stderr, fmt::fg(fmt::color::red), ": Failed to initialize GLFW.\n");
                break;
            case exception_t::window_creation_failed:
                fmt::print(stderr, fmt::fg(fmt::color::red), "[FATAL ERROR]: Failed to create the GLFW window.\n");
                break;
        }

        return EXIT_FAILURE;
    } catch (vulkan_creation_exception_t& exception) {
        fmt::print(
            stderr, 
            fmt::fg(fmt::color::red), 
            "[FATAL ERROR]: Failed to create a Vulkan {}. Vulkan error {}.\n", 
            exception.object_name, 
            static_cast<int>(exception.error_code)
        );

        return EXIT_FAILURE;
    } catch (no_adequate_physical_device_exception_t& exception) {
        fmt::print(
            stderr,
            fmt::fg(fmt::color::red),
            "[FATAL ERROR]: Could not find an adequate physical device.\n"
        );

        return EXIT_FAILURE;
    } catch (buffer_t::allocation_exception_t& exception) {
        fmt::print(
            stderr,
            fmt::fg(fmt::color::red),
            "[FATAL ERROR]: Could not allocate memory for a buffer: {}. Vulkan error {}",
            exception.what, static_cast<int>(exception.error_code)
        );
    }
}

