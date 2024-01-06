#pragma once

namespace pooper_cube {
    struct physical_device_t {
        VkPhysicalDevice handle;
        uint32_t graphics_queue_family;
        uint32_t present_queue_family;

        operator VkPhysicalDevice() const noexcept { return handle; }
    };

    class device_t {
        public:
            explicit device_t(const physical_device_t& physical_device);

            device_t(const device_t&) = delete;
            auto operator=(const device_t&) = delete;

            operator VkDevice() const noexcept { return m_device; }

            auto get_graphics_queue() const noexcept { return m_graphics_queue; }

            auto get_present_queue() const noexcept { return m_present_queue; }

            ~device_t() noexcept { vkDestroyDevice(m_device, nullptr); }

        private:
            VkDevice m_device;
            VkQueue m_graphics_queue;
            VkQueue m_present_queue;
    };

    struct no_adequate_physical_device_exception_t {};

    auto choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> physical_device_t; 
}
