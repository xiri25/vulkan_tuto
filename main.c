#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formats_count;
    VkPresentModeKHR* presentModes;
    uint32_t presentModes_count;
};

typedef struct vk_Struct {
    const uint32_t WIDTH;
    const uint32_t HEIGHT;

    const char** validationLayers;
    const uint32_t validationLayers_count;

    const char** deviceExtensions;
    const uint32_t deviceExtensions_count;

    const bool enableValidationLayers;

    GLFWwindow* window;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkDevice device;
 
    struct SwapChainSupportDetails swapChainSupport;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkSwapchainKHR swapChain;

    uint32_t imageCount;
    VkImage* swapChainImages;
    VkImageView* swapChainImageViews;
    VkFramebuffer* swapChainFramebuffers;

    VkRenderPass renderPass;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    const int MAX_FRAMES_IN_FLIGHT;
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    bool framebufferResized;

    VkBuffer vertexBuffer;
    uint64_t vertexBuffer_size;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    uint64_t indexBuffer_size;
    VkDeviceMemory indexBufferMemory;

    // NO me queda claro si es esto lo que hace...
    VkBuffer stagingBuffer;
    uint64_t stagingBuffer_size;
    VkDeviceMemory stagingBufferMemory;

    uint32_t currentFrame;
    
    void* vertices; // FIXME: Dont do it like this pls, no se como lo esta haciendo
    void* indices;
} vk_Struct_t;

struct Vertex {
    vec2 pos;
    vec3 color;
};

// NOTE: I dont like this
static VkVertexInputBindingDescription getBindingDescription()
{
    /*
     * A vertex binding describes at which rate to load data from memory
     * throughout the vertices. It specifies the number of bytes between
     * data entries and whether to move to the next data entry after each
     * vertex or after each instance.
     *
     * All of our per-vertex data is packed together in one array, so
     * we’re only going to have one binding. The binding parameter
     * specifies the index of the binding in the array of bindings.
     * The stride parameter specifies the number of bytes from one
     * entry to the next, and the inputRate parameter can have one
     * of the following values:
     *
     * VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
     * VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
     *
     * We’re not going to use instanced rendering, so we’ll stick to per-vertex data.
     */
    VkVertexInputBindingDescription result = {};
    result.binding = 0;
    result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    result.stride = sizeof(struct Vertex);

    return result;
}

static VkVertexInputAttributeDescription* getAttributeDescriptions()
{
    VkVertexInputAttributeDescription* result = malloc(sizeof(VkVertexInputAttributeDescription) * 2);
    result[0].location = 0;
    result[0].binding = 0;
    result[0].format = VK_FORMAT_R32G32_SFLOAT;
    result[0].offset = offsetof(struct Vertex, pos);

    result[1].location = 1;
    result[1].binding = 0;
    result[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    result[1].offset = offsetof(struct Vertex, color);

    return result;
}

/*
static struct VkVertexInputAttributeDescription_2 getAttributeDescriptions() {
    struct VkVertexInputAttributeDescription_2 result = {};
    result.pos.location = 0;
    result.pos.binding = 0;
    result.pos.format = VK_FORMAT_R32G32_SFLOAT;
    result.pos.offset = offsetof(struct Vertex, pos);

    result.color.location = 1;
    result.color.binding = 0;
    result.color.format = VK_FORMAT_R32G32B32_SFLOAT;
    result.color.offset = offsetof(struct Vertex, color);

    return result;
}
*/

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    //XD
    (void)width;
    (void)height;


    vk_Struct_t* app = (vk_Struct_t*)(glfwGetWindowUserPointer(window)); // XDDDDDDD
    app->framebufferResized = true;
}

GLFWwindow* create_window(vk_Struct_t* app)
{
    const uint32_t width = app->WIDTH;
    const uint32_t height = app->HEIGHT;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan", NULL, NULL);
    glfwSetWindowUserPointer(window, app); // TODO: IDK
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    return window;
}


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

void create_VkInstance(vk_Struct_t* app)
{
    if(app->enableValidationLayers && !checkValidationLayerSupport(app->validationLayers, app->validationLayers_count))
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
    if (app->enableValidationLayers)
    {
        createInfo.enabledLayerCount = app->validationLayers_count;
        createInfo.ppEnabledLayerNames = app->validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS)
    {
        printf("Failed to create an instance\n");
        exit(2);
    }
}

void createSurface(vk_Struct_t* app)
{
    if ( glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface) != VK_SUCCESS )
    {
        printf("Failed to create window surface\n");
        exit(7);
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
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    details.formats_count = formatCount; //For being able to know if is empty

    if (formatCount != 0)
    {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);
    details.presentModes_count = presentModeCount; //For being able to know if is empty

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

    bool isDeviceSuitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader && indices.graphicsFamily > -1 && // -1 porque el tuto usa optional
        extensionsSupported && swapChainAdequate;

    return isDeviceSuitable;
    /*
     * El tutorial implementa una forma de
     * ordenar distintas gpu para ver cual es mas suitable
     * en otra funcion, no lo voy a hacer
     */
}

void pickPhysicalDevice(vk_Struct_t* app)
{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!\n");
        exit(4);
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices);
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        if (isDeviceSuitable(devices[i], app->surface, app->deviceExtensions, app->deviceExtensions_count))
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

    app->physicalDevice = physicalDevice;
}

void createLogicalDevice(vk_Struct_t* app)
{
    VkDevice device;

    QueueFamilyIndices_t indices = findQueueFamilies(app->physicalDevice, app->surface);

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
    createInfo.enabledExtensionCount = app->deviceExtensions_count;
    createInfo.ppEnabledExtensionNames = app->deviceExtensions;
    if ( vkCreateDevice(app->physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS )
    {
        printf("failed to create logical device!\n");
        exit(6);
    }

    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &app->graphicsQueue);

    app->device = device;
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

void createSwapChain(vk_Struct_t* app)
{
    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(app->physicalDevice, app->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formats_count);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModes_count);
    app->swapChainExtent = chooseSwapExtent(&swapChainSupport.capabilities, app->window);

    /* 
     * Aside from these properties we also have to decide how many images we would like
     * to have in the swap chain. The implementation specifies the minimum number that it requires to function:
     * We should also make sure to not exceed the maximum number of images while
     * doing this, where 0 is a special value that means that there is no maximum:
     */

    app->imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount > 0 && app->imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        app->imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = app->surface;
    createInfo.minImageCount = app->imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = app->swapChainExtent;
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

    struct QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice, app->surface);
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

    if (vkCreateSwapchainKHR(app->device, &createInfo, NULL, &app->swapChain) != VK_SUCCESS)
    {
        printf("failed to create swap chain!\n");
        exit(8);
    }

    app->swapChainImageFormat = surfaceFormat.format;

    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);
}

void createImageViews(vk_Struct_t* app)
{
    for (size_t i = 0; i < app->imageCount; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = app->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = app->swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(app->device, &createInfo, NULL, &app->swapChainImageViews[i]) != VK_SUCCESS)
        {
            printf("failed to create image views!\n");
            exit(9);
        }
    }
}

void createRenderPass(vk_Struct_t* app)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = app->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    /*
      The loadOp and storeOp determine what to do with the data in the
      attachment before rendering and after rendering. We have the following choices for loadOp:

        - VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
        - VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
        - VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don’t care about them

      In our case we’re going to use the clear operation to clear the framebuffer
      to black before drawing a new frame. There are only two possibilities for the storeOp:

        - VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
        - VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operatio
     */

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(app->device, &renderPassInfo, NULL, &app->renderPass) != VK_SUCCESS) {
        printf("failed to create render pass!");
        exit(14);
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

void createGraphicsPipeline(vk_Struct_t* app)
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

    VkShaderModule vertShaderModule = createShaderModule(app->device, vertShaderCode, vertShaderSize);
    VkShaderModule fragShaderModule = createShaderModule(app->device, fragShaderCode, fragShaderSize);

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
     * member to NULL, which our struct initialization does automatically.
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

    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    VkVertexInputAttributeDescription* attributeDescriptions = getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

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
    viewport.width = (float) app->swapChainExtent.width;
    viewport.height = (float) app->swapChainExtent.height;
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
    scissor.extent = app->swapChainExtent;

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

    if (vkCreatePipelineLayout(app->device, &pipelineLayoutInfo, NULL, &app->pipelineLayout) != VK_SUCCESS) {
        printf("failed to create pipeline layout!\n");
        exit(13);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = app->pipelineLayout;
    pipelineInfo.renderPass = app->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    /* Here is where the shaders are compile form spir-v to machine expecific instructions */
    if (vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &app->graphicsPipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline!\n");
        exit(15);
    }

    /* cleanup */
    free(vertShaderCode);
    free(fragShaderCode);
    free(attributeDescriptions);
    vkDestroyShaderModule(app->device, vertShaderModule, NULL);
    vkDestroyShaderModule(app->device, fragShaderModule, NULL);
}

void createFramebuffers(vk_Struct_t* app)
{
    for (size_t i = 0; i < app->imageCount; i++) {
        VkImageView attachments[] = {
            app->swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = app->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = app->swapChainExtent.width;
        framebufferInfo.height = app->swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(app->device, &framebufferInfo, NULL, &app->swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            printf("failed to create framebuffer!\n");
            exit(16);
        }
    }
}

void createCommandPool(vk_Struct_t* app)
{
    /*
     * Commands in Vulkan, like drawing operations and memory transfers, are not executed
     * directly using function calls. You have to record all of the operations you want
     * to perform in command buffer objects. The advantage of this is that when we are
     * ready to tell the Vulkan what we want to do, all of the commands are submitted
     * together and Vulkan can more efficiently process the commands since all of
     * them are available together. In addition, this allows command recording to
     * happen in multiple threads if so desired.
     */

    QueueFamilyIndices_t queueFamilyIndices = findQueueFamilies(app->physicalDevice, app->surface); // XD

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(app->device, &poolInfo, NULL, &app->commandPool) != VK_SUCCESS)
    {
        printf("failed to create command pool!\n");
        exit(17);
    }
}

void createCommandBuffer(vk_Struct_t* app)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = app->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = app->MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(app->device, &allocInfo, app->commandBuffers) != VK_SUCCESS)
    {
        printf("failed to allocate command buffers!\n");
        exit(18);
    }
}

void createSyncObjects(vk_Struct_t* app)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Para no esperar indefinidamente la primera vez que se renderiza


    for (int32_t i = 0; i < app->MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &app->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &app->renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(app->device, &fenceInfo, NULL, &app->inFlightFences[i]) != VK_SUCCESS)
        {
            printf("failed to create semaphores!");
            exit(21);
        }
    }

}

uint32_t findMemoryType(vk_Struct_t* app, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    /*
     * Graphics cards can offer different types of memory to allocate from.
     * Each type of memory varies in terms of allowed operations and performance
     * characteristics. We need to combine the requirements of the buffer and our
     * own application requirements to find the right type of memory to use.
     */

    VkPhysicalDeviceMemoryProperties memProperties = {};
    vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memProperties);

    /*
     * The VkPhysicalDeviceMemoryProperties structure has two arrays memoryTypes
     * and memoryHeaps. Memory heaps are distinct memory resources like dedicated
     * VRAM and swap space in RAM for when VRAM runs out. The different types of
     * memory exist within these heaps. Right now we’ll only concern ourselves
     * with the type of memory and not the heap it comes from, but you can
     * imagine that this can affect performance.
     */

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    printf("failed to find suitable memory type!\n");
    exit(25);
}

void createBuffer(vk_Struct_t* app,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer* buffer,
                  VkDeviceMemory* bufferMemory)
{
    /*
     * Unlike the Vulkan objects we’ve been dealing
     * with so far, buffers do not automatically
     * allocate memory for themselves
     */

    VkBufferCreateInfo bufferInfo = {}; // TODO: Mejorar esta inicializacion
    //bufferInfo.flags = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(app->device, &bufferInfo, NULL, buffer);

    /*
     * The buffer has been created, but it doesn’t have any memory assigned
     * to it yet. The first step of allocating memory for the buffer is
     * to query its memory requirements using the aptly named
     * vkGetBufferMemoryRequirements function
     */

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements(app->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(app, memRequirements.memoryTypeBits, properties);

    VkResult result = vkAllocateMemory(app->device, &memoryAllocateInfo, NULL, bufferMemory);
    if (result != VK_SUCCESS) {
        printf("No se hga popido alojar memoria para el vertexBuffer\n");
        exit(26);
    }

    /*
     * The first three parameters are self-explanatory, and the fourth parameter is
     * the offset within the region of memory. Since this memory is allocated
     * specifically for this the vertex buffer, the offset is simply 0.
     * If the offset is non-zero, then it is required to be divisible by memRequirements.alignment.
     */
    vkBindBufferMemory(app->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(vk_Struct_t* app, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    /*
     * Memory transfer operations are executed using command buffers,
     * just like drawing commands. Therefore we must first allocate
     * a temporary command buffer. You may wish to create a separate
     * command pool for these kinds of short-lived buffers, because
     * the implementation may be able to apply memory allocation
     * optimizations. You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
     * flag during command pool generation in that case.
     */

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = app->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    VkCommandBuffer commandCopyBuffer;
    vkAllocateCommandBuffers(app->device, &allocInfo, &commandCopyBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandCopyBuffer, &beginInfo);
    VkBufferCopy bufferCopy = {};
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = size;
    vkCmdCopyBuffer(commandCopyBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

    vkEndCommandBuffer(commandCopyBuffer);

    // Execute the cmd buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandCopyBuffer;
    vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, NULL);

    vkQueueWaitIdle(app->graphicsQueue);
    
    /*
     * Unlike the draw commands, there are no events we
     * need to wait on this time. We just want to execute
     * the transfer on the buffers immediately. There are
     * again two possible ways to wait on this transfer
     * to complete. We could use a fence and wait with
     * vkWaitForFences, or simply wait for the transfer
     * queue to become idle with vkQueueWaitIdle. A
     * fence would allow you to schedule multiple transfers
     * simultaneously and wait for all of them complete,
     * instead of executing one at a time.
     * That may give the driver more opportunities to optimize.
     */
}

void createVertexBuffer(vk_Struct_t* app)
{
    /*
     * The vertex buffer we have right now works correctly,
     * but the memory type that allows us to access it from
     * the CPU may not be the most optimal memory type for
     * the graphics card itself to read from. The most
     * optimal memory has the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
     * flag and is usually not accessible by the CPU on dedicated
     * graphics cards. In this chapter, we’re going to create two
     * vertex buffers. One staging buffer in CPU accessible
     * memory to upload the data from the vertex array to,
     * and the final vertex buffer in device local memory.
     * We’ll then use a buffer copy command to move the
     * data from the staging buffer to the actual vertex buffer.
     */

    VkDeviceSize bufferSize = app->vertexBuffer_size;

    app->stagingBuffer_size = bufferSize;
    createBuffer(app,
                 app->stagingBuffer_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // TODO: Sharing mode exclusive, maybe not a default of createBUffer ???
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &app->stagingBuffer,
                 &app->stagingBufferMemory);
    // Buffer is already bind
    void* dataStaging;
    vkMapMemory(app->device, app->stagingBufferMemory, 0, bufferSize, (VkMemoryMapFlags)0 , &dataStaging);
    memcpy(dataStaging, app->vertices, (size_t)bufferSize);
    vkUnmapMemory(app->device, app->stagingBufferMemory);

    createBuffer(app,
                 bufferSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &app->vertexBuffer,
                 &app->vertexBufferMemory);
    // Buffer is already bind
    copyBuffer(app, app->stagingBuffer, app->vertexBuffer, bufferSize);

    /*
     * You can now simply memcpy the vertex data to the mapped memory
     * and unmap it again using vkUnmapMemory. Unfortunately, the driver
     * may not immediately copy the data into the buffer memory,
     * for example, because of caching. It is also possible that writes
     * to the buffer are not visible in the mapped memory yet. There
     * are two ways to deal with that problem:
     *
     * Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
     * Call vkFlushMappedMemoryRanges after writing to the mapped memory,
     * and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
     * We went for the first approach, which ensures that the mapped memory always
     * matches the contents of the allocated memory. Do keep in mind that this may lead to
     * slightly worse performance than explicit flushing, but we’ll see why that doesn’t matter in the next chapter.
     *
     * Flushing memory ranges or using a coherent memory heap means
     * that the driver will be aware of our writings to the buffer,
     * but it doesn’t mean that they are actually visible on the GPU
     * yet. The transfer of data to the GPU is an operation that
     * happens in the background, and the specification simply tells
     * us that it is guaranteed to be complete as of the next call to vkQueueSubmit.
     */
}

void createIndexBuffer(vk_Struct_t* app)
{
    VkDeviceSize bufferSize = app->indexBuffer_size;

    VkBuffer stagingBuffer = {};
    VkDeviceMemory stagingBufferMemory = {};
    createBuffer(app,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer,
                 &stagingBufferMemory);
    void* data;
    vkMapMemory(app->device, stagingBufferMemory, 0, bufferSize, (VkMemoryMapFlags)0 , &data);
    memcpy(data, app->indices, (size_t)bufferSize);

    createBuffer(app,
                 bufferSize,
                 VK_BUFFER_USAGE_2_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &app->indexBuffer,
                 &app->indexBufferMemory);

    copyBuffer(app, stagingBuffer, app->indexBuffer, bufferSize);
}

void initVulkan(vk_Struct_t* app)
{
    app->window = create_window(app);

    create_VkInstance(app);
    createSurface(app);
    pickPhysicalDevice(app);
    createLogicalDevice(app);

    createSwapChain(app);
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &app->imageCount, NULL); // NOTE: En la version original uso una query distinta para este imageCount
    app->swapChainImages = malloc(sizeof(VkImage) * app->imageCount);
    if (app->swapChainImages == NULL) {
        printf("No se  ha podido alojar memoria para swapChainImages\n");
        exit(1);
    }
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &app->imageCount, app->swapChainImages);

    app->swapChainImageViews = malloc(sizeof(VkImageView) * app->imageCount);
    if (app->swapChainImageViews == NULL) {
        printf("No se  ha podido alojar memoria para swapChainImageViews\n");
        exit(1);
    }
    createImageViews(app);

    createRenderPass(app);

    createGraphicsPipeline(app);

    app->swapChainFramebuffers = malloc(sizeof(VkFramebuffer) * app->imageCount);
    if (app->swapChainFramebuffers == NULL) {
        printf("No se  ha podido alojar memoria para swapChainFramebuffers\n");
        exit(1);
    }
    createFramebuffers(app);

    createCommandPool(app);

    createVertexBuffer(app);
    createIndexBuffer(app);

    app->commandBuffers = malloc(sizeof(VkCommandBuffer) * app->MAX_FRAMES_IN_FLIGHT);
    if (app->commandBuffers == NULL) {
        printf("No se  ha podido alojar memoria para commandBuffers\n");
        exit(1);
    }
    createCommandBuffer(app);

    app->imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * app->MAX_FRAMES_IN_FLIGHT);
    if (app->commandBuffers == NULL) {
        printf("No se  ha podido alojar memoria para imageAvailableSemaphores\n");
        exit(1);
    }
    app->renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * app->MAX_FRAMES_IN_FLIGHT);
    if (app->commandBuffers == NULL) {
        printf("No se  ha podido alojar memoria para renderFinishedSemaphores\n");
        exit(1);
    }
    app->inFlightFences = malloc(sizeof(VkFence) * app->MAX_FRAMES_IN_FLIGHT);
    if (app->commandBuffers == NULL) {
        printf("No se  ha podido alojar memoria para inFlightFences\n");
        exit(1);
    }
    createSyncObjects(app);
}

void recordCommandBuffer(vk_Struct_t* app, uint32_t imageIndex, uint32_t currentFrame)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = NULL; // Optional

    if (vkBeginCommandBuffer(app->commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        printf("failed to begin recording command buffer!\n");
        exit(19);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = app->renderPass;
    renderPassInfo.framebuffer = app->swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = app->swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(app->commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(app->commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphicsPipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)(app->swapChainExtent.width);
    viewport.height = (float)(app->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(app->commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = app->swapChainExtent;
    vkCmdSetScissor(app->commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(app->commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphicsPipeline);

    vkCmdBindVertexBuffers(app->commandBuffers[currentFrame], 0, 1, &app->vertexBuffer, (VkDeviceSize[]) {0});
    vkCmdBindIndexBuffer(app->commandBuffers[currentFrame], app->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    //vkCmdDraw(app->commandBuffers[currentFrame], 3, 1, 0, 0);
    vkCmdDrawIndexed(app->commandBuffers[currentFrame],
                     (app->indexBuffer_size / sizeof(uint16_t)),
                     1, // We are not using instancing
                     0, 0, 0);

    vkCmdEndRenderPass(app->commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(app->commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        printf("failed to record command buffer!\n");
        exit(20);
    }
}

void cleanupSwapChain(vk_Struct_t* app)
{
    for (uint32_t i = 0; i < app->imageCount; i++) {
        app->swapChainImageViews[i] = NULL;
    }

    app->swapChain = NULL;
}

void recreateSwapChain(vk_Struct_t* app)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(app->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(app->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(app->device);

    cleanupSwapChain(app);
    /*
     * Note that we don’t recreate the renderpass here for simplicity.
     * In theory, it can be possible for the swap chain image format
     * to change during an applications' lifetime, e.g., when moving
     * a window from a standard range to a high dynamic range monitor.
     * This may require the application to recreate the renderpass
     * to make sure the change between dynamic ranges is properly reflected.
     */

    createSwapChain(app);
    createImageViews(app);
}

void drawFrame(vk_Struct_t* app)
{
    /*
     * At a high level, rendering a frame in Vulkan consists of a common set of steps:
        - Wait for the previous frame to finish
        - Acquire an image from the swap chain
        - Record a command buffer which draws the scene onto that image
        - Submit the recorded command buffer
        - Present the swap chain image
     */

    uint32_t currentFrame = app->currentFrame;

    vkWaitForFences(app->device, 1, &app->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(app->device, app->swapChain, UINT64_MAX, app->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(app);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image!");
        exit(23);
    }
     
    /* Only reset the fence if we are submitting work */
    vkResetFences(app->device, 1, &app->inFlightFences[currentFrame]);

    vkResetCommandBuffer(app->commandBuffers[currentFrame], 0);
    recordCommandBuffer(app, imageIndex, currentFrame);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &app->commandBuffers[currentFrame];

    VkSemaphore waitSemaphores[] = {app->imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = app->commandBuffers;

    VkSemaphore signalSemaphores[] = {app->renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, app->inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        printf("failed to submit draw command buffer!\n");
        exit(22);
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {app->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL; // Optional

    result = vkQueuePresentKHR(app->graphicsQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        app->framebufferResized = false;
        recreateSwapChain(app);
    } else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image!\n");
        exit(24);
    }

    app->currentFrame = (currentFrame + 1) % app->MAX_FRAMES_IN_FLIGHT;
}

void mainLoop(vk_Struct_t* app)
{
    while (!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        drawFrame(app);
    }

    vkDeviceWaitIdle(app->device);
}

void cleanup(vk_Struct_t* app)
{
    for (size_t i = 0; i < 2; i++) {
        vkDestroySemaphore(app->device, app->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(app->device, app->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(app->device, app->inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(app->device, app->commandPool, NULL);

    for (uint32_t i = 0; i < app->imageCount; i++)
    {
        vkDestroyFramebuffer(app->device, app->swapChainFramebuffers[i], NULL);
    }

    vkDestroyPipeline(app->device, app->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipelineLayout, NULL);
    vkDestroyRenderPass(app->device, app->renderPass, NULL);

    for (uint32_t i = 0; i < app->imageCount; i++)
    {
        vkDestroyImageView(app->device, app->swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(app->device, app->swapChain, NULL);

    vkDestroyDevice(app->device, NULL);

    vkDestroySurfaceKHR(app->instance, app->surface, NULL);

    vkDestroyInstance(app->instance, NULL);

    glfwDestroyWindow(app->window);

    glfwTerminate();

    // TODO: No se si tengo que free() los mallocs que hago en initVulkan
}

int main(void)
{
    vk_Struct_t App = {
        .WIDTH = 800,
        .HEIGHT = 600,
        .MAX_FRAMES_IN_FLIGHT = 2,
        .validationLayers_count = 1,
        .validationLayers = malloc(sizeof(const char*) * 1),
#ifdef NDEBUG
        .enableValidationLayers = false,
#else
        .enableValidationLayers = true,
#endif
        .deviceExtensions = malloc(sizeof(const char*) * 1),
        .deviceExtensions_count = 1,
        .framebufferResized = false,
    };

    if (App.validationLayers == NULL) {
        printf("No se ha popido alojar memoria para las validationLayers\n");
        return 1;
    }
    App.validationLayers[0] = "VK_LAYER_KHRONOS_validation";
    App.deviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME; // Its a fucking macro, no a string WTF?????

    const struct Vertex vertices[4] = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    App.vertexBuffer_size = sizeof(vertices[0]) * 4;
    App.vertices = (void*)&vertices[0];

    const uint16_t indices[6] = {
        0, 1, 2, 2, 3, 0
    };

    App.indexBuffer_size = sizeof(indices[0]) * 6;
    App.indices = (void*)&indices[0];

    App.currentFrame = 0;

    initVulkan(&App);

    mainLoop(&App);

    cleanup(&App);

    return 0;
}
