#pragma once

#include "common.hpp"
#include "devices.hpp"
#include "commands.hpp"

namespace pooper_cube {
    struct vertex_t {
        glm::vec3 position;
    };

    static constexpr std::array<VkVertexInputAttributeDescription, 1> vertex_attribute_descriptions {
        VkVertexInputAttributeDescription {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(vertex_t, position),
        },
    };

    class buffer_t {
        public:
            enum class type_t {
                vertex, element, staging, uniform
            };

            struct allocation_exception_t {
                VkResult error_code;
                std::string_view what;
            };

            buffer_t(const physical_device_t& physical_device, const device_t& device, type_t type, VkDeviceSize size);
            NO_COPY(buffer_t);

            operator VkBuffer() const noexcept {
                return m_buffer;
            }

            auto copy_from(const buffer_t& source, const command_pool_t& command_pool) const -> void;

            virtual ~buffer_t() noexcept {
                vkFreeMemory(m_device, m_memory, nullptr);
                vkDestroyBuffer(m_device, m_buffer, nullptr);
            }

        protected:
            const device_t& m_device;

            VkBuffer m_buffer;
            VkDeviceMemory m_memory;
            VkDeviceSize m_size;
    };

    class host_coherent_buffer_t : public buffer_t {
        public:
            host_coherent_buffer_t(const physical_device_t& physical_device, const device_t& device, type_t type, VkDeviceSize size)
                : buffer_t(physical_device, device, type, size)
            {}

            class mapped_memory_t {
                public:
                    mapped_memory_t(void* data, const host_coherent_buffer_t& buffer) :
                        m_data(data), m_buffer(buffer) {}
                    NO_COPY(mapped_memory_t);

                    using data_type_t = void*;
                    operator data_type_t() const noexcept { return m_data; }

                    ~mapped_memory_t() noexcept {
                        vkUnmapMemory(m_buffer.m_device, m_buffer.m_memory);
                    }

                private:
                    void* m_data;
                    const host_coherent_buffer_t& m_buffer;
            };

            auto map_memory() const noexcept -> mapped_memory_t {
                void* data;
                vkMapMemory(m_device, m_memory, 0, m_size, 0, &data);
                return mapped_memory_t{data, *this};
            }
    };

    auto find_memory_type(const physical_device_t& p_physical_device, uint32_t p_type_filter, VkMemoryPropertyFlags properties) -> std::optional<uint32_t>; 
}
