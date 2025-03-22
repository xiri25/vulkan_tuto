//#include <vulkan/vulkan.h>

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

void cleanup(GLFWwindow* window, VkInstance instance, VkDevice device)
{
    vkDestroyDevice(device, NULL);

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
} QueueFamilyIndices_t;

QueueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device)
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
        .graphicsFamily = -1
    };
    
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
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
    }

    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    QueueFamilyIndices_t indices;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    indices = findQueueFamilies(device); // El tuto ahora solo hace esta comprobacion XD

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader && indices.graphicsFamily > -1; // -1 porque el tuto usa optional

    /*
     * El tutorial implementa una forma de
     * ordenar distintas gpu para ver cual es mas suitable
     * en otra funcion, no lo voy a hacer
     */
}

VkPhysicalDevice pickPhysicalDevice(VkInstance vk_instance)
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
        if (isDeviceSuitable(devices[i]))
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

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkQueue* graphicsQueue)
{
    VkDevice device;

    QueueFamilyIndices_t indices = findQueueFamilies(physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    if ( vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS )
    {
        printf("failed to create logical device!\n");
        exit(6);
    }

    vkGetDeviceQueue(device, indices.graphicsFamily, 0, graphicsQueue); // TODO: API figure it out

    return device;
}

/* TODO: ENUM for error for exit() */
int main(void)
{

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    const uint32_t validationLayers_count = 1;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    GLFWwindow* window = create_window(WIDTH, HEIGHT);

    /* TODO: figure out the API, todo esto es initVulkan() en el tuto */
    VkInstance instance;
    create_VkInstance(&instance, validationLayers, validationLayers_count, enableValidationLayers);

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance);

    VkQueue graphicsQueue = {};
    VkDevice device = createLogicalDevice(physicalDevice, &graphicsQueue);

    main_loop(window);

    cleanup(window, instance, device);

    return 0;
}
