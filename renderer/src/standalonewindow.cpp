#include "standalonewindow.h"

#include <SDL.h>
#include <SDL_vulkan.h>

StandaloneWindow::StandaloneWindow(Renderer* renderer) : m_renderer(renderer) {
    SDL_Init(SDL_INIT_EVERYTHING);

    m_window = SDL_CreateWindow("mdlviewer viewport", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_VULKAN);
}

VkSurfaceKHR StandaloneWindow::getSurface(VkInstance instance) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    SDL_Vulkan_CreateSurface(m_window, instance, &surface);
    return surface;
}

void StandaloneWindow::render() {
    m_renderer->render(models);
}
