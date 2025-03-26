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

void cleanup(GLFWwindow* window, VkInstance instance, VkDevice device, VkSurfaceKHR surface, VkSwapchainKHR swapchain, VkImageView* imageViews, const uint32_t imageCount)
{
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

    main_loop(window);

    cleanup(window, instance, device, surface, swapChain, swapChainImageViews, imageCount);

    return 0;
}
