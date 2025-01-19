#ifndef SWAPCHAIN_CHOICES_H
#define SWAPCHAIN_CHOICES_H

#include <vector>
#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

#endif //SWAPCHAIN_CHOICES_H
