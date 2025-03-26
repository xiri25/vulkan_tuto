#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* La he tenido que traducir de cpp porque usaba for loops fancy XD, puede estar mal */
bool checkValidationLayerSupport(const char** validationLayers, const uint32_t validationLayers_count)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    /*
    printf("Capas de validación detectadas por el programa:\n");
    for (uint32_t i = 0; i < layerCount; i++) {
        printf("- %s\n", availableLayers[i].layerName);
    }
    */

    for (uint32_t j = 0; j < validationLayers_count; j++)
    {
        bool layerFound = false;

        for (uint32_t i = 0; i < layerCount; i++)
        {
            if (strcmp(validationLayers[j], availableLayers[i].layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void createSurface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR* surface)
{
    if ( glfwCreateWindowSurface(instance, window, NULL, surface) != VK_SUCCESS )
    {
        printf("Failed to create window surface\n");
        exit(7);
    }
}

GLFWwindow* create_window(const uint32_t width, const uint32_t height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(width, height, "Vulkan", NULL, NULL);
}

void main_loop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void cleanup(GLFWwindow* window, VkInstance instance, VkDevice device, VkSurfaceKHR surface, VkSwapchainKHR swapchain, VkImageView* imageViews, const uint32_t imageCount, VkPipelineLayout pipelineLayout)
{
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);

    for (uint32_t i = 0; i < imageCount; i++)
    {
        vkDestroyImageView(device, imageViews[i], NULL);
    }

    vkDestroySwapchainKHR(device, swapchain, NULL);

    vkDestroyDevice(device, NULL);

    vkDestroySurfaceKHR(instance, surface, NULL);

    vkDestroyInstance(instance, NULL);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void create_VkInstance(VkInstance* vk_instance, const char** validationLayers, const uint32_t validationLayers_count, const bool enableValidationLayers)
{
    if(enableValidationLayers && !checkValidationLayerSupport(validationLayers, validationLayers_count))
    {
        printf("validation layers requested, but not available\n");
        exit(3);
    }

    VkApplicationInfo appInfo = {}; //Importante inicializar
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {}; //Importante inicializar
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = NULL;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensions) {
        printf("GLFW failed to get required Vulkan extensions!\n");
        exit(1);
    }

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = validationLayers_count;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, NULL, vk_instance) != VK_SUCCESS)
    {
        printf("Failed to create an instance\n");
        exit(2);
    }
}

typedef struct QueueFamilyIndices {
    int32_t graphicsFamily; // We dont have optional, so negative means that is not found
    int32_t presentFamily;
} QueueFamilyIndices_t;

QueueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    /* 
     * There are different types of queues
     * that originate from different queue
     * families and each family of queues
     * allows only a subset of commands.
     * For example, there could be a queue
     * family that only allows processing
     * of compute commands or one that only
     * allows memory transfer related commands.
     * We need to check which queue families are
     * supported by the device and which one of
     * these supports the commands that we want
     * to use. For that purpose we’ll add a new
     * function findQueueFamilies that looks
     * for all the queue families we need.
    */
    QueueFamilyIndices_t indices = {
        .graphicsFamily = -1,
        .presentFamily = -1
    };
    
    VkBool32 presentSupport = false;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    /*
     * The VkQueueFamilyProperties struct contains
     * some details about the queue family,
     * including the type of operations that are
     * supported and the number of queues that
     * can be created based on that family. We need
     * to find at least one queue family that
     * supports VK_QUEUE_GRAPHICS_BIT
    */

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (presentSupport)
        {
            indices.presentFamily = i;
        }
    }

    return indices;
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formats_count;
    VkPresentModeKHR* presentModes;
    uint32_t presentModes_count;
};

struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    struct SwapChainSupportDetails details = {};
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    details.formats_count = formatCount; //For being able to know if is empty
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    if (formatCount != 0)
    {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    details.presentModes_count = presentModeCount; //For being able to know if is empty
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    if (presentModeCount != 0)
    {
        details.presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes);
    }

    return details;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const char** deviceExtensions, const uint32_t deviceExtensionsCount)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    // TODO: I dont like this GPT shit

    // Create a dynamic array to store required extensions
    char** requiredExtensions = (char**)malloc(deviceExtensionsCount * sizeof(char*));
    size_t requiredCount = deviceExtensionsCount;

    for (size_t i = 0; i < deviceExtensionsCount; i++) {
        requiredExtensions[i] = strdup(deviceExtensions[i]);
    }

    // Check available extensions and remove from required
    for (size_t i = 0; i < extensionCount; i++) {
        for (size_t j = 0; j < requiredCount; j++) {
            if (requiredExtensions[j] != NULL && strcmp(requiredExtensions[j], availableExtensions[i].extensionName) == 0) {
                free(requiredExtensions[j]);
                requiredExtensions[j] = NULL; // Mark as removed
            }
        }
    }

    // Check if all required extensions are removed
    bool isEmpty = true;
    for (size_t i = 0; i < requiredCount; i++) {
        if (requiredExtensions[i] != NULL) {
            isEmpty = false;
            free(requiredExtensions[i]); // Clean up remaining elements
        }
    }

    free(requiredExtensions);
    return isEmpty;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, const uint32_t availableFormats_count)
{
    for (uint32_t i = 0; i < availableFormats_count; i++)
    {
        if (availableFormats->format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormats[i];
        }
    }

    /* If not found the one we want, return the first */
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, const uint32_t availablePresentModes_count) {
    /*
      The presentation mode is arguably the most important setting for the swap chain,
      because it represents the actual conditions for showing images to the screen.
      There are four possible modes available in Vulkan:
        - VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are
        transferred to the screen right away, which may result in tearing.

        - VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes
        an image from the front of the queue when the display is refreshed and the
        program inserts rendered images at the back of the queue. If the queue is full
        then the program has to wait. This is most similar to vertical sync as found
        in modern games. The moment that the display is refreshed is known as "vertical blank".

        - VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous
        one if the application is late and the queue was empty at the last vertical
        blank. Instead of waiting for the next vertical blank, the image is
        transferred right away when it finally arrives. This may result in visible tearing.

        - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode.
        Instead of blocking the application when the queue is full, the images that
        are already queued are simply replaced with the newer ones. This mode can be
        used to render frames as fast as possible while still avoiding tearing, resulting
        in fewer latency issues than standard vertical sync. This is commonly known as
        "triple buffering", although the existence of three buffers alone does not
        necessarily mean that the framerate is unlocked.
        
        Only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available,
        so we’ll again have to write a function that looks for the best mode that is available
     */

    for (uint32_t i = 0; i < availablePresentModes_count; i++)
    {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

double clamp_uint32_t(uint32_t d, uint32_t min, uint32_t max)
{
    /* https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value */
    const double t = d < min ? min : d;
    return t > max ? max : t;
}


VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* window)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            (uint32_t)width,
            (uint32_t)height
        };

        actualExtent.width = clamp_uint32_t(actualExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        actualExtent.height = clamp_uint32_t(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

        return actualExtent;
    }
}

void createSwapChain(VkSwapchainKHR* swapChain, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window, VkDevice device, VkFormat* format, VkExtent2D* extent)
{
    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formats_count);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModes_count);
    *extent = chooseSwapExtent(&swapChainSupport.capabilities, window);

    /* 
     * Aside from these properties we also have to decide how many images we would like
     * to have in the swap chain. The implementation specifies the minimum number that it requires to function:
     * We should also make sure to not exceed the maximum number of images while
     * doing this, where 0 is a special value that means that there is no maximum:
     */

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = *extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /*
     * The imageArrayLayers specifies the amount of layers each image
     * consists of. This is always 1 unless you are developing a
     * stereoscopic 3D application. The imageUsage bit field specifies
     * what kind of operations we’ll use the images in the swap chain
     * for. In this tutorial we’re going to render directly to them,
     * which means that they’re used as color attachment. It is also
     * possible that you’ll render images to a separate image first
     * to perform operations like post-processing. In that case you
     * may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead
     * and use a memory operation to transfer the rendered image
     * to a swap chain image
     */

    struct QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    /*
     * Next, we need to specify how to handle swap chain images that
     * will be used across multiple queue families. That will be the
     * case in our application if the graphics queue family is different
     * from the presentation queue. We’ll be drawing on the images
     * in the swap chain from the graphics queue and then submitting
     * them on the presentation queue. There are two ways to handle
     * images that are accessed from multiple queues:
        - VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue
          family at a time and ownership must be explicitly
          transferred before using it in another queue family. This
          option offers the best performance.
        - VK_SHARING_MODE_CONCURRENT: Images can be used across
          multiple queue families without explicit ownership transfers.
     * If the queue families differ, then we’ll be using the concurrent
     * mode in this tutorial to avoid having to do the ownership chapters,
     * because these involve some concepts that are better explained
     * at a later time. Concurrent mode requires you to specify in advance
     * between which queue families ownership will be shared using the
     * queueFamilyIndexCount and pQueueFamilyIndices parameters.
     * If the graphics queue family and presentation queue family are
     * the same, which will be the case on most hardware, then we should
     * stick to exclusive mode, because concurrent mode requires you to
     * specify at least two distinct queue families.
     */

    /*
     * We can specify that a certain transform should be applied to
     * images in the swap chain if it is supported (supportedTransforms in capabilities),
     * like a 90 degree clockwise rotation or horizontal flip. To specify
     * that you do not want any transformation, simply specify the current transformation.
     */
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    /*
     * The compositeAlpha field specifies if the alpha channel should be
     * used for blending with other windows in the window system.
     * You’ll almost always want to simply ignore the alpha channel,
     * hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
     */
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; /* Dont care about color of obscured pixels, best performance
                                    I think it is for when miminized or behind another window */
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, NULL, swapChain) != VK_SUCCESS)
    {
        printf("failed to create swap chain!\n");
        exit(8);
    }

    *format = surfaceFormat.format;

    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const char** deviceExtensions, const uint32_t deviceExtensionsCount)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    QueueFamilyIndices_t indices;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    indices = findQueueFamilies(device, surface); // El tuto ahora solo hace esta comprobacion XD
    
    bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions, deviceExtensionsCount);

    /* 
     * It is important that we only try to query
     * for swap chain support after verifying that
     * the extension is available
     */

    bool swapChainAdequate = false;
    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
    if (extensionsSupported)
    {
        swapChainAdequate = (swapChainSupport.formats_count != 0 && swapChainSupport.presentModes_count != 0);
    }

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader && indices.graphicsFamily > -1 && // -1 porque el tuto usa optional
           extensionsSupported && swapChainAdequate;
    /*
     * El tutorial implementa una forma de
     * ordenar distintas gpu para ver cual es mas suitable
     * en otra funcion, no lo voy a hacer
     */
}

VkPhysicalDevice pickPhysicalDevice(VkInstance vk_instance, VkSurfaceKHR surface, const char** deviceExtensions, const uint32_t deviceExtensionsCount)
{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!\n");
        exit(4);
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, devices);
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        if (isDeviceSuitable(devices[i], surface, deviceExtensions, deviceExtensionsCount))
        {
            physicalDevice = devices[i]; // Elige el primero que vale
            break;
        }
    }
// -1 porque no hay optional, TODO: I hate it
    
    if (physicalDevice == VK_NULL_HANDLE)
    {
        printf("failed to find a suitable GPU!\n");
        exit(5);
    }

    return physicalDevice;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkQueue* graphicsQueue, VkSurfaceKHR surface, const char** deviceExtensions, const uint32_t deviceExtensions_count)
{
    VkDevice device;

    QueueFamilyIndices_t indices = findQueueFamilies(physicalDevice, surface);

    uint32_t uniqueQueueFamilies[2] = {indices.graphicsFamily, indices.presentFamily};
    uint32_t queueFamilyCount = (indices.graphicsFamily == indices.presentFamily) ? 1 : 2;

    VkDeviceQueueCreateInfo queueCreateInfos[queueFamilyCount];
    float queuePriority = 1.0f;

    /* I think this shit works XD */
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfos[i] = queueCreateInfo;
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueFamilyCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions_count;
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    if ( vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS )
    {
        printf("failed to create logical device!\n");
        exit(6);
    }

    vkGetDeviceQueue(device, indices.graphicsFamily, 0, graphicsQueue); // TODO: API figure it out

    return device;
}

void createImageViews(VkImage* swapChainImages, const uint32_t swapChainImages_count, const VkFormat swapChainImageFormat ,VkImageView* swapChainImageViews, VkDevice device)
{
    for (size_t i = 0; i < swapChainImages_count; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            printf("failed to create image views!\n");
            exit(9);
        }
    }
}

char* readFile(const char* filename, size_t* fileSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("failed to open file %s\n", filename);
        exit(10);
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(*fileSize);
    if (!buffer) {
        printf("failed to allocate memory for the file %s\n", filename);
        fclose(file);
        exit(11);
    }

    fread(buffer, 1, *fileSize, file);
    fclose(file);

    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const char* code, size_t size)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = (uint32_t*)code;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)
    {
        printf("failed to create shader module!");
        exit(12);
    }

    return shaderModule;
}

void CreateGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkPipelineLayout* pipelineLayout)
{
    /*
     * The graphics pipeline in Vulkan is almost completely immutable,
     * so you must recreate the pipeline from scratch if you want to
     * change shaders, bind different framebuffers or change the blend
     * function. The disadvantage is that you’ll have to create a number
     * of pipelines that represent all of the different combinations of
     * states you want to use in your rendering operations. However,
     * because all of the operations you’ll be doing in the pipeline
     * are known in advance, the driver can optimize for it much better
     *
     * 
        - The input assembler collects the raw vertex data from the
          buffers you specify and may also use an index buffer to
          repeat certain elements without having to duplicate the vertex data itself.

        - The vertex shader is run for every vertex and generally applies
          transformations to turn vertex positions from model space to
          screen space. It also passes per-vertex data down the pipeline.

        - The tessellation shaders allow you to subdivide geometry based
          on certain rules to increase the mesh quality. This is often
          used to make surfaces like brick walls and staircases look
          less flat when they are nearby.

        - The geometry shader is run on every primitive (triangle, line, point)
          and can discard it or output more primitives than came in.
          This is similar to the tessellation shader, but much more flexible.
          However, it is not used much in today’s applications because
          the performance is not that good on most graphics cards except
          for Intel’s integrated GPUs.

        - The rasterization stage discretizes the primitives into fragments.
          These are the pixel elements that they fill on the framebuffer.
          Any fragments that fall outside the screen are discarded and
          the attributes outputted by the vertex shader are interpolated
          across the fragments, as shown in the figure. Usually the
          fragments that are behind other primitive fragments are also
          discarded here because of depth testing.

        - The fragment shader is invoked for every fragment that survives
          and determines which framebuffer(s) the fragments are written to
          and with which color and depth values. It can do this using the
          interpolated data from the vertex shader, which can include
          things like texture coordinates and normals for lighting.

        - The color blending stage applies operations to mix different
          fragments that map to the same pixel in the framebuffer.
          Fragments can simply overwrite each other, add up or be mixed
          based upon transparency.

     */

    size_t vertShaderSize = 0;
    char* vertShaderCode = readFile("shaders/vert.spv", &vertShaderSize);
    size_t fragShaderSize = 0;
    char* fragShaderCode = readFile("shaders/frag.spv", &fragShaderSize);

    VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode, vertShaderSize);
    VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode, fragShaderSize);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    /*
     * The next two members specify the shader module containing the code,
     * and the function to invoke, known as the entrypoint. That means that
     * it’s possible to combine multiple fragment shaders into a single
     * shader module and use different entry points to differentiate between
     * their behaviors. In this case we’ll stick to the standard main, however.
     */
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    /*
     * There is one more (optional) member, pSpecializationInfo, which we won’t
     * be using here, but is worth discussing. It allows you to specify values
     * for shader constants. You can use a single shader module where its
     * behavior can be configured at pipeline creation by specifying different
     * values for the constants used in it. This is more efficient than
     * configuring the shader using variables at render time, because the compiler
     * can do optimizations like eliminating if statements that depend on these
     * values. If you don’t have any constants like that, then you can set the
     * member to nullptr, which our struct initialization does automatically.
     */

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    /* FIXED FUNCTIONS */

    /*
     * While most of the pipeline state needs to be baked into the pipeline state,
     * a limited amount of the state can actually be changed without recreating
     * the pipeline at draw time. Examples are the size of the viewport,
     * line width and blend constants. If you want to use dynamic state and
     * keep these properties out, then you’ll have to fill in a
     * VkPipelineDynamicStateCreateInfo structure like this:
     */
    const uint32_t dynamicStates_count = 2;
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates_count;
    dynamicState.pDynamicStates = dynamicStates;


    /*
        The VkPipelineVertexInputStateCreateInfo structure describes the format of
        the vertex data that will be passed to the vertex shader.
        It describes this in roughly two ways:
            - Bindings: spacing between data and whether the data is per-vertex
              or per-instance (see instancing)
            
            - Attribute descriptions: type of the attributes passed to the vertex
              shader, which binding to load them from and at which offset
        
        Because we’re hard coding the vertex data directly in the vertex shader,
        we’ll fill in this structure to specify that there is no vertex data to
        load for now. We’ll get back to it in the vertex buffer chapter.
     */

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL; // Optional


    /* Input Assembly */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    /* Viewports and Scissors */
    /*
     * A viewport basically describes the region of the framebuffer that the output
     * will be rendered to. This will almost always be (0, 0) to (width, height)
     * and in this tutorial that will also be the case.
     */
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    /*
     * While viewports define the transformation from the image to the framebuffer,
     * scissor rectangles define in which regions pixels will actually be stored.
     * Any pixels outside the scissor rectangles will be discarded by the rasterizer.
     */
    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapChainExtent;

    /*
     * Viewport(s) and scissor rectangle(s) can either be specified as a static part
     * of the pipeline or as a dynamic state set in the command buffer. While the former
     * is more in line with the other states it’s often convenient to make viewport
     * and scissor state dynamic as it gives you a lot more flexibility. This is very
     * common and all implementations can handle this dynamic state without a performance penalty.
     */
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    /* Rasterizer */
    /*
     * The rasterizer takes the geometry that is shaped by the vertices from the vertex
     * shader and turns it into fragments to be colored by the fragment shader.
     * It also performs depth testing, face culling and the scissor test, and it can
     * be configured to output fragments that fill entire polygons or just the edges
     * (wireframe rendering). All this is configured using the
     * VkPipelineRasterizationStateCreateInfo structure.
     */
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;

    /*
     * If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes
     * through the rasterizer stage. This basically disables any output to the framebuffer.
     */
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    /*
     * The polygonMode determines how fragments are generated for geometry. The following modes are available:
        VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
        VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
        VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
        Using any mode other than fill requires enabling a GPU feature.
     */
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


    /* Multisamplig (gpu feature to enable) */
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional


    /* Color blending */
    /*
        After a fragment shader has returned a color, it needs to be
        combined with the color that is already in the framebuffer.
        This transformation is known as color blending and there are
        two ways to do it:

        - Mix the old and new value to produce a final color
        - Combine the old and new value using a bitwise operation

        There are two types of structs to configure color blending.
        The first struct, VkPipelineColorBlendAttachmentState contains
        the configuration per attached framebuffer and the second struct,
        VkPipelineColorBlendStateCreateInfo contains the global
        color blending settings. In our case we only have one framebuffer:
     */
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
    /*
     * It is also possible to disable both modes, as we’ve done here,
     * in which case the fragment colors will be written to the
     * framebuffer unmodified
     */

    /* Pipeline Layout */
    /*
     * You can use uniform values in shaders, which are globals
     * similar to dynamic state variables that can be changed
     * at drawing time to alter the behavior of your shaders
     * without having to recreate them. They are commonly used
     * to pass the transformation matrix to the vertex shader,
     * or to create texture samplers in the fragment shader.
     * These uniform values need to be specified during pipeline
     * creation by creating a VkPipelineLayout object. Even though
     * we won’t be using them until a future chapter,
     * we are still required to create an empty pipeline layout.
     */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = NULL; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, pipelineLayout) != VK_SUCCESS) {
        printf("failed to create pipeline layout!\n");
        exit(12);
    }


    /* cleanup */
    free(vertShaderCode);
    free(fragShaderCode);
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
}

/* TODO: ENUM for error for exit() */
int main(void)
{

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    const uint32_t validationLayers_count = 1;

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME // Its a fucking macro, no a string WTF?????
    };
    const uint32_t deviceExtensions_count = 1;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    GLFWwindow* window = create_window(WIDTH, HEIGHT);

    /* TODO: figure out the API, todo esto es initVulkan() en el tuto */
    VkInstance instance;
    create_VkInstance(&instance, validationLayers, validationLayers_count, enableValidationLayers);

    VkSurfaceKHR surface;
    createSurface(instance, window, &surface);

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface, deviceExtensions, deviceExtensions_count);

    VkQueue graphicsQueue = {};
    VkDevice device = createLogicalDevice(physicalDevice, &graphicsQueue, surface, deviceExtensions, deviceExtensions_count);

    /* Creacion de la swapchain */
    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    VkFormat swapChainImageFormat = {};
    VkExtent2D swapChainExtent = {};

    VkSwapchainKHR swapChain = {};
    createSwapChain(&swapChain, physicalDevice, surface, window, device, &swapChainImageFormat, &swapChainExtent);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    VkImage swapChainImages[imageCount];
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);
    /* Fin de la creacion de la swapchain */

    /* Creacion de las images views */
    VkImageView swapChainImageViews[imageCount];
    createImageViews(swapChainImages, imageCount, swapChainImageFormat, swapChainImageViews, device);

    /* Creacion de la Graphics Pipeline */
    VkPipelineLayout pipelineLayout;
    CreateGraphicsPipeline(device, swapChainExtent, &pipelineLayout);

    main_loop(window);

    cleanup(window, instance, device, surface, swapChain, swapChainImageViews, imageCount, pipelineLayout);

    return 0;
}
