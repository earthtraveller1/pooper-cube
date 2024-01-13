#include "buffers.hpp"
#include "commands.hpp"
#include "descriptors.hpp"
#include "devices.hpp"
#include "images.hpp"
#include "pipelines.hpp"
#include "swapchain.hpp"
#include "sync-objects.hpp"
#include "vulkan-debug.hpp"
#include "vulkan-instance.hpp"

namespace {
    using pooper_cube::vertex_t;

    enum class cube_side_t {
        front, back, top, bottom, left, right
    };

    auto append_cube_face(std::vector<vertex_t>& p_vertices, std::vector<uint32_t>& p_indices, float p_size, cube_side_t p_side) {
        struct pairs_t {
            float a;
            float b;
        };

        const std::array<pairs_t, 4> pairs {
            pairs_t {0.5f * p_size, -0.5f * p_size},
            pairs_t {0.5f * p_size, 0.5f * p_size},
            pairs_t {-0.5f * p_size, 0.5f * p_size},
            pairs_t {-0.5f * p_size, -0.5f * p_size}
        };

        for (auto pair : pairs) {
            switch (p_side) {
                case cube_side_t::front:
                    p_vertices.push_back(vertex_t {
                        .position = {pair.a, pair.b, p_size * 0.5f}
                    });
                    break;
                case cube_side_t::back:
                    p_vertices.push_back(vertex_t {
                        .position = {pair.a, pair.b, p_size * -0.5f}
                    });
                    break;
                case cube_side_t::right:
                    p_vertices.push_back(vertex_t {
                        .position = {0.5f * p_size, pair.b, -pair.a}
                    });
                    break;
                case cube_side_t::left:
                    p_vertices.push_back(vertex_t {
                        .position = {-0.5f * p_size, pair.b, pair.a}
                    });
                    break;
                case cube_side_t::top:
                    p_vertices.push_back(vertex_t {
                        .position = {pair.a, 0.5f * p_size, pair.b}
                    });
                    break;
                case cube_side_t::bottom:
                    p_vertices.push_back(vertex_t {
                        .position = {pair.a, -0.5f * p_size, pair.b}
                    });
                    break;
            }
        }

        const auto index_base = static_cast<uint32_t>(p_vertices.size());

        const auto is_back_side = p_side == cube_side_t::back |
                                  p_side == cube_side_t::bottom |
                                  p_side == cube_side_t::left;

        if (is_back_side) {
            p_indices.insert(p_indices.end(), {
                index_base + 0,
                index_base + 3,
                index_base + 2,
                index_base + 2,
                index_base + 1,
                index_base + 0
           });
        } else {
            p_indices.insert(p_indices.end(), {
                index_base + 0,
                index_base + 1,
                index_base + 2,
                index_base + 0,
                index_base + 3,
                index_base + 2
            });
        }
    }
}

auto main(int p_argc, char** p_argv) -> int {
    using pooper_cube::buffer_t;
    using pooper_cube::choose_physical_device;
    using pooper_cube::command_pool_t;
    using pooper_cube::debug_messenger_t;
    using pooper_cube::descriptor_layout_t;
    using pooper_cube::descriptor_pool_t;
    using pooper_cube::device_t;
    using pooper_cube::fence_t;
    using pooper_cube::framebuffers_t;
    using pooper_cube::generic_vulkan_exception_t;
    using pooper_cube::graphics_pipeline_t;
    using pooper_cube::host_coherent_buffer_t;
    using pooper_cube::image_t;
    using pooper_cube::instance_t;
    using pooper_cube::no_adequate_physical_device_exception_t;
    using pooper_cube::pipeline_layout_t;
    using pooper_cube::render_pass_t;
    using pooper_cube::semaphore_t;
    using pooper_cube::shader_module_t;
    using pooper_cube::swapchain_t;
    using pooper_cube::vulkan_creation_exception_t;
    using pooper_cube::window_t;

    struct push_constants_t {
        glm::mat4 model;
        float color_offset;
    };

    struct uniform_buffer_object_t {
        glm::mat4 view;
        glm::mat4 projection;
        float color_offset;
    };

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
        swapchain_t swapchain{window, physical_device, logical_device, window_surface};

        const command_pool_t command_pool{logical_device, physical_device.graphics_queue_family};

        const shader_module_t vertex_shader{logical_device, shader_module_t::type_t::vertex, "shaders/triangle.vert.spv"};
        const shader_module_t fragment_shader{logical_device, shader_module_t::type_t::fragment, "shaders/triangle.frag.spv"};

        const descriptor_layout_t descriptor_layout{logical_device, std::array<VkDescriptorSetLayoutBinding, 1> {
            VkDescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr
            }
        }};

        const std::array<VkDescriptorSetLayout, 1> set_layouts{descriptor_layout};
        const std::array<VkPushConstantRange, 1> push_constant_ranges {
            VkPushConstantRange {
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(push_constants_t),
            }
        };

        const descriptor_pool_t descriptor_pool{
            logical_device, 
            std::array<VkDescriptorPoolSize, 1> {
                VkDescriptorPoolSize {
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
                    .descriptorCount = 1
                }
            }, 
            1
        };

        const auto descriptor_set = descriptor_pool.allocate_set(descriptor_layout);
        const host_coherent_buffer_t uniform_buffer{physical_device, logical_device, buffer_t::type_t::uniform, sizeof(uniform_buffer_object_t)};
        const auto uniform_buffer_address = uniform_buffer.map_memory();

        {
            const VkDescriptorBufferInfo buffer_info{
                .buffer = uniform_buffer,
                .offset = 0,
                .range = sizeof(uniform_buffer_object_t),
            };

            const VkWriteDescriptorSet descriptor_write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &buffer_info,
                .pTexelBufferView = nullptr,
            };
            
            vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
        }

        pooper_cube::image_t depth_buffer{
            physical_device, 
            logical_device, 
            swapchain.get_extent().width, 
            swapchain.get_extent().height, 
            pooper_cube::image_t::type_t::depth_buffer
        };

        const pipeline_layout_t pipeline_layout{logical_device, set_layouts, push_constant_ranges};
        const render_pass_t render_pass{logical_device, swapchain.get_format(), pooper_cube::find_depth_format(physical_device).value()};
        const graphics_pipeline_t graphics_pipeline{logical_device, vertex_shader, fragment_shader, pipeline_layout, render_pass};

        framebuffers_t framebuffers{logical_device, swapchain, depth_buffer, render_pass};

        const buffer_t vertex_buffer{physical_device, logical_device, buffer_t::type_t::vertex, 4*sizeof(pooper_cube::vertex_t)};

        {
            const pooper_cube::vertex_t vertices[] {
                {{0.5f, -0.5f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}},
                {{-0.5f, -0.5f, 0.0f}}
            };

            const pooper_cube::host_coherent_buffer_t staging_buffer{physical_device, logical_device, buffer_t::type_t::staging, sizeof(vertices)};

            {
                const auto memory = staging_buffer.map_memory();
                std::memcpy(memory, vertices, sizeof(vertices));
            }

            vertex_buffer.copy_from(staging_buffer, command_pool);
        }

        const buffer_t index_buffer{physical_device, logical_device, buffer_t::type_t::element, 6*sizeof(uint32_t)};

        {
            const uint32_t indices[] {
                0, 1, 2,
                0, 2, 3
            };

            const pooper_cube::host_coherent_buffer_t staging_buffer{physical_device, logical_device, buffer_t::type_t::staging, sizeof(indices)};

            {
                const auto memory = staging_buffer.map_memory();
                std::memcpy(memory, indices, sizeof(indices));
            }

            index_buffer.copy_from(staging_buffer, command_pool);
        }

        const semaphore_t acquired_image_semaphore{logical_device}, rendering_done_semaphore{logical_device};
        const fence_t rendering_done_fence{logical_device};

        const auto command_buffer = command_pool.allocate_command_buffer();

        window.show();
        while (!window.should_close()) {
            const VkFence rendering_done_fence_raw = rendering_done_fence;
            const auto frame_time = glfwGetTime();

            VkResult result;
#define VK_ERROR(f, m) result = f; if (result != VK_SUCCESS) { throw generic_vulkan_exception_t{result, m}; }

            VK_ERROR(
                vkWaitForFences(logical_device, 1, &rendering_done_fence_raw, VK_TRUE, std::numeric_limits<uint64_t>::max()),
                "Failed to wait for fences"
            );
            uint32_t image_index;
            
            result = vkAcquireNextImageKHR(logical_device, swapchain, std::numeric_limits<uint64_t>::max(), acquired_image_semaphore, VK_NULL_HANDLE, &image_index);
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                VK_ERROR(vkDeviceWaitIdle(logical_device), "Failed to wait for the device to complete operations.");
                // The old swap chain must be destroyed before replacing it with a new one, and that 
                // is done in this case by setting it to a null swap chain. Same thing with the framebuffers.
                framebuffers = framebuffers_t{logical_device};
                depth_buffer = image_t{logical_device};
                swapchain = swapchain_t{logical_device}; 
                swapchain = swapchain_t{window, physical_device, logical_device, window_surface};
                depth_buffer = image_t{physical_device, logical_device, swapchain.get_extent().width, swapchain.get_extent().height, image_t::type_t::depth_buffer};
                framebuffers = framebuffers_t{logical_device, swapchain, depth_buffer, render_pass};
                continue;
            } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                throw generic_vulkan_exception_t{result, "Failed to retrieve an image from the swap chain."};
            }

            VK_ERROR(
                vkResetFences(logical_device, 1, &rendering_done_fence_raw),
                "Failed to reset the fences!"
            );

            VK_ERROR(vkResetCommandBuffer(command_buffer, 0), "Failed to reset the command buffer!");

            const VkCommandBufferBeginInfo command_buffer_begin_info {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pInheritanceInfo = nullptr,
            };

            VK_ERROR(
                vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info),
                "Failed to start recording the command buffer!"
            );

            const std::array<VkClearValue, 2> clear_values {
                VkClearValue {
                    .color = {
                        .float32 = {
                            0.0f, 0.0f, 0.0f, 1.0f
                        }
                    }
                },
                VkClearValue {
                    .depthStencil = {
                        .depth = 1.0f,
                        .stencil = 0,
                    }
                }
            };

            const VkRenderPassBeginInfo render_pass_begin_info {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = render_pass,
                .framebuffer = framebuffers.get(image_index),
                .renderArea = {
                    .offset = {
                        .x = 0,
                        .y = 0,
                    },
                    .extent = swapchain.get_extent()
                },
                .clearValueCount = clear_values.size(),
                .pClearValues = clear_values.data(),
            };
            
            vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

            const VkViewport viewport {
                .x = 0,
                .y = 0,
                .width = static_cast<float>(swapchain.get_extent().width),
                .height = static_cast<float>(swapchain.get_extent().height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };

            vkCmdSetViewport(command_buffer, 0, 1, &viewport);

            const VkRect2D scissor {
                .offset = {
                    .x = 0,
                    .y = 0,
                },
                .extent = swapchain.get_extent()
            };

            vkCmdSetScissor(command_buffer, 0, 1, &scissor);

            const VkDeviceSize offset = 0;
            const VkBuffer vertex_buffer_raw = vertex_buffer;
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer_raw, &offset);
            vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const VkDescriptorSet descriptor_set_raw = descriptor_set;
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set_raw, 0, nullptr);

            const push_constants_t push_constants {
                .model = glm::rotate(glm::mat4{1.0f}, glm::radians(static_cast<float>(frame_time*50.0f)), glm::vec3{1.0f, 0.5f, 0.0f}),
                .color_offset = static_cast<float>(std::sin(frame_time) / 2 + 0.5),
            };

            vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_constants_t), &push_constants);

            const auto window_dimensions = window.get_dimensions();
            const auto aspect_ratio = static_cast<float>(window_dimensions.width) / static_cast<float>(window_dimensions.height);

            const uniform_buffer_object_t uniform_buffer_object {
                .view = glm::translate(glm::mat4{1.0f} , glm::vec3{0.0f, 0.0f, -2.0f}),
                .projection = glm::perspective(glm::radians(70.0f), aspect_ratio, 0.01f, 100.0f),
                .color_offset = static_cast<float>(std::cos(frame_time) / 2 + 0.5),
            };

            memcpy(uniform_buffer_address, &uniform_buffer_object, sizeof(uniform_buffer_object));

            vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);

            vkCmdEndRenderPass(command_buffer);

            VK_ERROR(
                vkEndCommandBuffer(command_buffer),
                "Failed to stop recording the command buffer"
            );

            const VkSemaphore acquired_image_semaphore_raw = acquired_image_semaphore;
            const VkSemaphore rendering_done_semaphore_raw = rendering_done_semaphore;

            const VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

            const VkSubmitInfo submit_info {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &acquired_image_semaphore_raw,
                .pWaitDstStageMask = wait_stages,
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &rendering_done_semaphore_raw
            };

            VK_ERROR(
                vkQueueSubmit(logical_device.get_graphics_queue(), 1, &submit_info, rendering_done_fence),
                "Failed to submit the command buffer to the graphics queue!"
            );

            const VkSwapchainKHR swapchain_raw = swapchain;

            const VkPresentInfoKHR present_info {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &rendering_done_semaphore_raw,
                .swapchainCount = 1,
                .pSwapchains = &swapchain_raw,
                .pImageIndices = &image_index,
                .pResults = nullptr,
            };

            result = vkQueuePresentKHR(logical_device.get_present_queue(), &present_info);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                vkDeviceWaitIdle(logical_device);
                // The old swap chain must be destroyed before replacing it with a new one, and that 
                // is done in this case by setting it to a null swap chain. Same thing with the framebuffers.
                framebuffers = framebuffers_t{logical_device};
                depth_buffer = image_t{logical_device};
                swapchain = swapchain_t{logical_device}; 
                swapchain = swapchain_t{window, physical_device, logical_device, window_surface};
                depth_buffer = image_t{physical_device, logical_device, swapchain.get_extent().width, swapchain.get_extent().height, image_t::type_t::depth_buffer};
                framebuffers = framebuffers_t{logical_device, swapchain, depth_buffer, render_pass};
            } else if (result != VK_SUCCESS) {
                throw generic_vulkan_exception_t{result, "Failed to present to the swap chain."};
            }

#undef VK_ERROR
            window.poll_events();
        }

        vkDeviceWaitIdle(logical_device);
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
    } catch (const pooper_cube::generic_vulkan_exception_t& exception) {
        fmt::print(
            stderr,
            fmt::fg(fmt::color::red),
            "[VULKAN ERROR {}]: {}\n",
            static_cast<int>(exception.error_code), exception.what
        );

        return EXIT_FAILURE;
    }
}

