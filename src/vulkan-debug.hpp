#pragma once

namespace pooper_cube {
    class debug_messenger_t {
        public:
            explicit debug_messenger_t(VkInstance instance);

            debug_messenger_t(const debug_messenger_t&) = delete;
            auto operator=(const debug_messenger_t&) -> debug_messenger_t& = delete;

            debug_messenger_t(debug_messenger_t&& source) noexcept;
            auto operator=(debug_messenger_t&& right_hand_side) noexcept -> debug_messenger_t&;

            ~debug_messenger_t() noexcept;
        private:
            VkDebugUtilsMessengerEXT m_handle;
            VkInstance m_instance;
    };

    extern const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO;
}
