#include "buffers.hpp"
#include "commands.hpp"
#include "devices.hpp"
#include "pipelines.hpp"
#include "swapchain.hpp"
#include "sync-objects.hpp"
#include "vulkan-debug.hpp"
#include "vulkan-instance.hpp"

auto main(int p_argc, char** p_argv) -> int {
    using pooper_cube::choose_physical_device;
    using pooper_cube::command_pool_t;
    using pooper_cube::debug_messenger_t;
    using pooper_cube::device_t;
    using pooper_cube::instance_t;
    using pooper_cube::no_adequate_physical_device_exception_t;
    using pooper_cube::generic_vulkan_exception_t;
    using pooper_cube::shader_module_t;
    using pooper_cube::swapchain_t;
    using pooper_cube::vulkan_creation_exception_t;
    using pooper_cube::window_t;
    using pooper_cube::pipeline_layout_t;
    using pooper_cube::graphics_pipeline_t;
    using pooper_cube::buffer_t;
    using pooper_cube::semaphore_t;
    using pooper_cube::fence_t;

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

        const std::vector<VkDescriptorSetLayout> set_layouts;
        const std::vector<VkPushConstantRange> push_constant_ranges;

        const pipeline_layout_t pipeline_layout{logical_device, set_layouts, push_constant_ranges};
        const graphics_pipeline_t graphics_pipeline{logical_device, vertex_shader, fragment_shader, pipeline_layout};

        const buffer_t vertex_buffer{physical_device, logical_device, buffer_t::type_t::vertex, 3*sizeof(pooper_cube::vertex_t)};

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

        const semaphore_t acquired_image_semaphore{logical_device}, rendering_done_semaphore{logical_device};
        const fence_t rendering_done_fence{logical_device};

        const auto command_buffer = command_pool.allocate_command_buffer();

        window.show();
        while (!window.should_close()) {
            const VkFence rendering_done_fence_raw = rendering_done_fence;

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
                // is done in this case by setting it to a null swap chain.
                swapchain = swapchain_t{logical_device}; 
                swapchain = swapchain_t{window, physical_device, logical_device, window_surface};
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

            const VkImageMemoryBarrier image_barrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = swapchain.get_image(image_index),
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            vkCmdPipelineBarrier(
                command_buffer, 
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                0, 
                0, nullptr, 
                0, nullptr, 
                1, &image_barrier
            );

            const VkRenderingAttachmentInfo attachment_info {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = swapchain.get_image_view(image_index),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = static_cast<VkResolveModeFlagBits>(0),
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = static_cast<VkImageLayout>(0),
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                    .color = {
                        .float32 = {
                            0.0f, 0.0f, 0.0f, 1.0f
                        }
                    }
                }
            };

            const VkRenderingInfo rendering_info {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderArea = {
                    .offset = {
                        .x = 0,
                        .y = 0,
                    },
                    .extent = swapchain.get_extent(),
                },
                .layerCount = 1,
                .viewMask = 0,
                .colorAttachmentCount = 1,
                .pColorAttachments = &attachment_info,
                .pDepthAttachment = nullptr,
                .pStencilAttachment = nullptr
            };

            vkCmdBeginRendering(command_buffer, &rendering_info);

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

            vkCmdDraw(command_buffer, 3, 1, 0, 0);

            vkCmdEndRendering(command_buffer);

            const VkImageMemoryBarrier image_barrier_2 {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = 0,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = swapchain.get_image(image_index),
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            vkCmdPipelineBarrier(
                command_buffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                0, 
                0, nullptr, 
                0, nullptr, 
                1, &image_barrier_2
            );
            
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
                // is done in this case by setting it to a null swap chain.
                swapchain = swapchain_t{logical_device}; 
                swapchain = swapchain_t{window, physical_device, logical_device, window_surface};
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

