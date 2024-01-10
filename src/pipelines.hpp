#pragma once

#include "common.hpp"
#include "devices.hpp"

namespace pooper_cube {
    class shader_module_t {
        public:
            enum class type_t {
                vertex, fragment
            };

            explicit shader_module_t(const device_t& device, type_t type, std::string_view code_path);

            auto get_shader_stage() const noexcept -> VkPipelineShaderStageCreateInfo {
                return {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = m_type,
                    .module = m_module,
                    .pName = "main",
                };
            }

            auto get_type() const noexcept {
                return m_type;
            }

            ~shader_module_t() noexcept {
                vkDestroyShaderModule(m_device, m_module, nullptr);
            }
            
        private:
            const device_t& m_device;

            VkShaderModule m_module;
            VkShaderStageFlagBits m_type;
    };

    class pipeline_layout_t {
        public:
            pipeline_layout_t(const device_t& device, std::span<const VkDescriptorSetLayout> set_layouts, std::span<const VkPushConstantRange> push_constant_ranges);
            NO_COPY(pipeline_layout_t);

            operator VkPipelineLayout() const noexcept { return m_layout; }

            ~pipeline_layout_t() noexcept {
                vkDestroyPipelineLayout(m_device, m_layout, nullptr);
            }

        private:
            const device_t& m_device;

            VkPipelineLayout m_layout;
    };

    class render_pass_t {
        public:
            render_pass_t(const device_t& device, VkFormat format);
            NO_COPY(render_pass_t);

            operator VkRenderPass() const noexcept { return m_render_pass; }

            ~render_pass_t() noexcept {
                vkDestroyRenderPass(m_device, m_render_pass, nullptr);
            }

        private:
            const device_t& m_device;

            VkRenderPass m_render_pass;
    };

    class graphics_pipeline_t {
        public:
            graphics_pipeline_t(const device_t& device, const shader_module_t& vertex_module, const shader_module_t& fragment_module, const pipeline_layout_t& layout);
            NO_COPY(graphics_pipeline_t);

            operator VkPipeline() const noexcept { return m_pipeline; }

            ~graphics_pipeline_t() noexcept {
                vkDestroyPipeline(m_device, m_pipeline, nullptr);
            }
        private:
            VkPipeline m_pipeline;
            const device_t& m_device;
    };
}
